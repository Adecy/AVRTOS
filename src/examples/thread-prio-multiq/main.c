#include <avrtos/kernel.h>
#include <avrtos/drivers/usart.h>


int main(void)
{
	const struct usart_config cfg = USART_CONFIG_DEFAULT_500000();
	usart_drv_init(USART0_DEVICE, &cfg);

	uint32_t u = 0u;
	for (;;) {
		printf_P(PSTR("Hello ! %u\n"), u++);

		k_sleep(K_SECONDS(1));
	}
}