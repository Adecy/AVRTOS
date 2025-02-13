/*
 * Copyright (c) 2024 Lucas Dietrich <ld.adecy@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "i2c.h"

#include <avrtos/drivers.h>
#include <avrtos/drivers/gpio.h>
#include <avrtos/misc/serial.h>
#include <avrtos/semaphore.h>

#include <avr/eeprom.h>
#include <util/twi.h>

#if CONFIG_I2C_DRIVER_ENABLE

#if MCU_I2C_COUNT == 0
#error "No I2C device enabled"
#endif

#if defined(I2C0_DEVICE) && CONFIG_I2C0_ENABLED
#define I2C0_DEVICE_ENABLED 1
#define I2C0_INDEX			0
#else
#define I2C0_DEVICE_ENABLED 0
#endif

#define FREQ_CALC(_prescaler, _twbr)                                                     \
	(F_CPU / (16 + 2 * (_twbr)*I2C_PRESCALER_VALUE(_prescaler)))
#define FREQ_MIN FREQ_CALC(I2C_PRESCALER_64, 255)
#define FREQ_MAX 400000lu

#if defined(I2C1_DEVICE) && CONFIG_I2C1_ENABLED
#define I2C1_DEVICE_ENABLED 1
// Adjust I2C1 index if I2C0 is not enabled
#if MCU_I2C_COUNT == 1
#define I2C1_INDEX 0
#elif MCU_I2C_COUNT == 2
#define I2C1_INDEX 1
#endif
#else
#define I2C1_DEVICE_ENABLED 0
#endif

#define I2C_MAX_BUF_LEN ((1u << CONFIG_I2C_MAX_BUF_LEN_BITS) - 1u)

#define TWI_INT_MASK	(CONFIG_I2C_INTERRUPT_DRIVEN ? BIT(TWIE) : 0u)
#define TWI_ENABLE_MASK (BIT(TWINT) | BIT(TWEN) | TWI_INT_MASK)

#define TWI_START(_dev)		  _dev->TWCRn = TWI_ENABLE_MASK | BIT(TWSTA);
#define TWI_STOP(_dev)		  _dev->TWCRn = TWI_ENABLE_MASK | BIT(TWSTO)
#define TWI_RESET(_dev)		  _dev->TWCRn = TWI_ENABLE_MASK | BIT(TWSTO) | BIT(TWSTA)
#define TWI_REPLY(_dev, _ack) _dev->TWCRn = TWI_ENABLE_MASK | (_ack ? BIT(TWEA) : 0u)

typedef enum {
	/* Driver is uninitialized */
	UNINITIALIZED = 0,
	/* Driver is initialized and ready to use */
	READY = 1,
	/* Driver is currently transmitting (busy) */
	MASTER_TX = 2,
	/* Driver is currently receiving (busy) */
	MASTER_RX = 3,
} i2c_state_t;

struct i2c_context {
	uint8_t *buf;
	uint8_t sla_w; // Address already shifted by 1 + write bit

	/* Generic cursor to keep track */
	uint8_t cursor : CONFIG_I2C_MAX_BUF_LEN_BITS;

	/* State of the I2C state machine */
	i2c_state_t state : 2u;

	/* Length to write */
	uint8_t write_len : CONFIG_I2C_MAX_BUF_LEN_BITS;

	/* Length to read */
	uint8_t read_len : CONFIG_I2C_MAX_BUF_LEN_BITS;

#if CONFIG_I2C_LAST_ERROR
	i2c_error_t error;
#endif // CONFIG_I2C_LAST_ERROR
};

#if CONFIG_I2C_LAST_ERROR
#define set_error(_x, _err) (_x)->error = _err
#define get_error(_x)		(_x)->error
#else
#define set_error(_x, _err)
#define get_error(_x) I2C_ERROR_NONE
#endif // CONFIG_I2C_LAST_ERROR

static struct i2c_context i2c_contexts[MCU_I2C_COUNT];

static int8_t i2c_get_device_index(const I2C_Device *dev)
{
	switch ((uint16_t)dev) {
#if I2C0_DEVICE_ENABLED
	case I2C0_BASE_ADDR:
		return I2C0_INDEX;
#endif // I2C0_DEVICE_ENABLED
#if I2C1_DEVICE_ENABLED
	case I2C1_BASE_ADDR:
		return I2C1_INDEX;
#endif // I2C1_DEVICE_ENABLED
	default:
		return -EBADF;
	}
}

static struct i2c_context *i2c_get_context(const I2C_Device *dev)
{
	const int8_t index = i2c_get_device_index(dev);
	if (index < 0) return NULL;

	return &i2c_contexts[index];
}

/* Section 26.2.1 of ATmega328PB datasheet  */
static void prr_enable(uint8_t dev_index)
{
	switch (dev_index) {
#if I2C0_DEVICE_ENABLED
	case I2C0_INDEX:
		PRR0 &= ~BIT(PRTWI0);
		break;
#endif // I2C0_DEVICE_ENABLED
#if I2C1_DEVICE_ENABLED
	case I2C1_INDEX:
		PRR1 &= ~BIT(PRTWI1);
		break;
#endif // I2C1_DEVICE_ENABLED
	default:
		break;
	}
}

static void i2c_gpio_setup(uint8_t dev_index, bool enable)
{
	uint8_t pullup = enable ? GPIO_INPUT_PULLUP : GPIO_INPUT_NO_PULLUP;

	switch (dev_index) {
#if I2C0_DEVICE_ENABLED
	case I2C0_INDEX:
		gpiol_pin_init(GPIOC_DEVICE, 4u, GPIO_INPUT, pullup);
		gpiol_pin_init(GPIOC_DEVICE, 5u, GPIO_INPUT, pullup);
		break;
#endif // I2C0_DEVICE_ENABLED
#if I2C1_DEVICE_ENABLED
	case I2C1_INDEX:
		gpiol_pin_init(GPIOE_DEVICE, 0u, GPIO_INPUT, pullup);
		gpiol_pin_init(GPIOE_DEVICE, 1u, GPIO_INPUT, pullup);
		break;
#endif // I2C1_DEVICE_ENABLED
	default:
		break;
	}
}

int8_t i2c_init(I2C_Device *dev, struct i2c_config config)
{
	int8_t ret;
	int8_t dev_index;

	dev_index = i2c_get_device_index(dev);
	if (dev_index < 0) {
		ret = -EINVAL;
		goto exit;
	}

	dev->TWCRn = 0u; // Disable device

	// configure device
	dev->TWSRn	= config.prescaler;
	dev->TWBRn	= config.twbr;
	dev->TWARn	= 0u;
	dev->TWAMRn = 0u;

	prr_enable(dev_index);

	// set internal pullups on SDA, SCL
	i2c_gpio_setup(dev_index, true);

	dev->TWCRn					  = BIT(TWINT) | BIT(TWEN); // Enable device and interrupt
	i2c_contexts[dev_index].state = READY;
	ret							  = 0;

exit:
	return ret;
}

int8_t i2c_deinit(I2C_Device *dev)
{
	int8_t ret;
	int8_t dev_index;

	dev_index = i2c_get_device_index(dev);
	if (dev_index < 0) {
		ret = -EINVAL;
		goto exit;
	}

	if (i2c_contexts[dev_index].state != READY) {
		ret = -EBUSY;
		goto exit;
	}

	dev->TWCRn	= 0u;
	dev->TWBRn	= 0u;
	dev->TWSRn	= 0u;
	dev->TWARn	= 0x02;
	dev->TWDRn	= 0x01;
	dev->TWAMRn = 0u;

	// clear internal pullups on SDA, SCL
	i2c_gpio_setup(dev_index, false);

	i2c_contexts[dev_index].state = UNINITIALIZED;
	ret							  = 0;

exit:
	return ret;
}

void transfer_stop(I2C_Device *dev, struct i2c_context *x)
{
	TWI_STOP(dev);
	x->state = READY;
}

__always_inline void poll_end(struct i2c_context *x)
{
	do {
		memory_barrier();
	} while (x->state != READY);
}

__always_inline void poll_irq(I2C_Device *dev)
{
	while (!(dev->TWCRn & BIT(TWINT)))
		;
}

__always_inline void i2c_state_machine(I2C_Device *dev, struct i2c_context *x)
{
	const uint8_t status = dev->TWSRn & TW_STATUS_MASK;

#if CONFIG_I2C_DEBUG
	serial_hex(status);
	serial_print("\n");
#endif

	switch (status) {
	case TW_START: // Start condition transmitted
	case TW_REP_START:
		dev->TWDRn = x->sla_w;
		TWI_REPLY(dev, 1u);
		/* Next expected interrupt is TW_MT_SLA_ACK */
		break;

	case TW_MT_DATA_ACK: // Data transmitted, ACK received
		x->cursor++;
	case TW_MT_SLA_ACK: // SLA+W transmitted, ACK received
		if (x->cursor < x->write_len) {
			dev->TWDRn = x->buf[x->cursor++];
			TWI_REPLY(dev, 1u);
			/* Next expected interrupt is TW_MT_DATA_ACK */
		} else if (x->read_len != 0) {
			/* Switch to reception */
			x->state  = MASTER_RX;
			x->cursor = 0u;
			x->sla_w  = (x->sla_w & ~TW_WRITE) | TW_READ;
			TWI_START(dev); /* Trigger a repeated start */
							/* Next expected interrupt is TW_REP_START */
		} else {
			transfer_stop(dev, x);
			/* Transmission complete */
		}
		break;

	case TW_MR_DATA_ACK: // Data received, ACK returned
		x->buf[x->cursor++] = dev->TWDRn;
	case TW_MR_SLA_ACK: // SLA+R transmitted, ACK received
		/*  ACK if more data is expected (NACK otherwise) */
		TWI_REPLY(dev, x->read_len - x->cursor > 1u);
		break;

	case TW_MR_DATA_NACK: // Data received, NACK returned
		x->buf[x->cursor++] = dev->TWDRn;
		transfer_stop(dev, x);
		/* Reception complete */
		break;

	case TW_MT_SLA_NACK:  // SLA+W transmitted, NACK received
	case TW_MT_DATA_NACK: // Data transmitted, NACK received
		set_error(x, (status == TW_MT_SLA_NACK) ? I2C_ERROR_ADDR : I2C_ERROR_DATA);
		transfer_stop(dev, x);
		/* Transmission complete with error */
		break;

	case TW_MT_ARB_LOST:
		// case TW_MR_ARB_LOST:
		TWI_START(dev);
		break;

	case TW_MR_SLA_NACK:
		set_error(x, I2C_ERROR_ADDR);
		transfer_stop(dev, x);
		/* Reception complete with error */
		break;

	// fatal
	case TW_NO_INFO:
	case TW_BUS_ERROR:
	default:
		set_error(x, I2C_ERROR_BUS);
		transfer_stop(dev, x);
		break;
	}
}

__always_inline void i2c_state_machine_loop(I2C_Device *dev, struct i2c_context *x)
{
	do {
		poll_irq(dev);
		i2c_state_machine(dev, x);
		memory_barrier();
	} while (x->state != READY);
}

static int8_t
i2c_run(I2C_Device *dev, uint8_t addr, uint8_t *data, uint8_t w_len, uint8_t r_len)
{
	int8_t ret					= 0;
	struct i2c_context *const x = i2c_get_context(dev);

	Z_ARGS_CHECK(x && data && (w_len <= I2C_MAX_BUF_LEN) && (r_len <= I2C_MAX_BUF_LEN))
	{
		return -EINVAL;
	}
	if (x->state != READY) return -EBUSY;

	x->sla_w = (addr << 1);
	if (w_len == 0) {
		x->state = MASTER_RX;
		x->sla_w |= TW_READ;
	} else {
		x->state = MASTER_TX;
		x->sla_w |= TW_WRITE;
	}
	x->buf		 = data;
	x->write_len = w_len;
	x->read_len	 = r_len;
	x->cursor	 = 0u;
	set_error(x, I2C_ERROR_NONE);

	TWI_START(dev);

#if !CONFIG_I2C_INTERRUPT_DRIVEN
	switch ((uint16_t)dev) {
#if I2C0_DEVICE_ENABLED
	case I2C0_BASE_ADDR:
		i2c_state_machine_loop(I2C0_DEVICE, x);
		break;
#endif
#if I2C1_DEVICE_ENABLED
	case I2C1_BASE_ADDR:
		i2c_state_machine_loop(I2C1_DEVICE, x);
		break;
#endif
	default:
		break;
	}
	if (get_error(x) != I2C_ERROR_NONE) ret = -EIO;
#elif CONFIG_I2C_BLOCKING
	poll_end(x);
	if (get_error(x) != I2C_ERROR_NONE) ret = -EIO;
#endif

	return ret;
}

int8_t i2c_master_write(I2C_Device *dev, uint8_t addr, const uint8_t *data, uint8_t len)
{
	return i2c_run(dev, addr, (uint8_t *)data, len, 0u);
}

int8_t i2c_master_read(I2C_Device *dev, uint8_t addr, uint8_t *data, uint8_t len)
{
	return i2c_run(dev, addr, data, 0u, len);
}

int8_t i2c_master_write_read(
	I2C_Device *dev, uint8_t addr, uint8_t *data, uint8_t wlen, uint8_t rlen)
{
	return i2c_run(dev, addr, data, wlen, rlen);
}

int8_t i2c_status(I2C_Device *dev)
{
	struct i2c_context *const x = i2c_get_context(dev);
	Z_ARGS_CHECK(x) return -EINVAL;
	if (x->state != READY) return -EBUSY;
	return 0;
}

i2c_error_t i2c_poll_end(I2C_Device *dev)
{
	struct i2c_context *const x = i2c_get_context(dev);
	Z_ARGS_CHECK(x) return I2C_ERROR_ARGS;

#if CONFIG_I2C_INTERRUPT_DRIVEN
	poll_end(x);
#endif

	return get_error(x);
}

i2c_error_t i2c_last_error(I2C_Device *dev)
{
	struct i2c_context *const x = i2c_get_context(dev);
	Z_ARGS_CHECK(x) return I2C_ERROR_ARGS;
	return get_error(x);
}

int8_t i2c_calc_config(struct i2c_config *config, uint32_t desired_freq)
{
	ARG_UNUSED(desired_freq);
	ARG_UNUSED(config);
	Z_ARGS_CHECK(config) return -EINVAL;

	return -ENOTSUP;
}

#if CONFIG_I2C_INTERRUPT_DRIVEN
#if I2C0_DEVICE_ENABLED
ISR(TWI0_vect)
{
	i2c_state_machine(I2C0_DEVICE, &i2c_contexts[I2C0_INDEX]);
}
#endif // I2C0_DEVICE_ENABLED

#if I2C1_DEVICE_ENABLED
ISR(TWI1_vect)
{
	i2c_state_machine(I2C0_DEVICE, &i2c_contexts[I2C1_INDEX]);
}
#endif // I2C1_DEVICE_ENABLED
#endif // CONFIG_I2C_INTERRUPT_DRIVEN
#endif // CONFIG_I2C_DRIVER_ENABLE