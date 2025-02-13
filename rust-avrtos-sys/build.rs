const LINKER_SCRIPT: &'static str = "architecture/avr/avrtos-avr6.xn";

const SOURCES: &[&str] = &[
    "../src/avrtos/arch/arch_atomic.S",
    "../src/avrtos/arch/arch_utils.S",
    "../src/avrtos/arch/debug_utils.S",
    "../src/avrtos/arch/init_sections.S",
    "../src/avrtos/arch/thread_switch_preempt.S",

    "../src/avrtos/devices/tcn75.c",
    "../src/avrtos/devices/mcp2515.c",

    "../src/avrtos/drivers/exti.c",
    "../src/avrtos/drivers/gpio.c",
    "../src/avrtos/drivers/i2c.c",
    "../src/avrtos/drivers/spi.c",
    "../src/avrtos/drivers/timer.c",
    "../src/avrtos/drivers/usart.c",

    "../src/avrtos/dstruct/debug.c",
    "../src/avrtos/dstruct/dlist.c",
    "../src/avrtos/dstruct/slist.c",
    "../src/avrtos/dstruct/tqueue.c",
    "../src/avrtos/dstruct/tdqueue.c",

    "../src/avrtos/misc/led.c",
    "../src/avrtos/misc/serial.c",

    "../src/avrtos/assert.c",
    "../src/avrtos/atomic.c",
    "../src/avrtos/canaries.c",
    "../src/avrtos/debug.c",
    "../src/avrtos/event.c",
    "../src/avrtos/fault.c",
    "../src/avrtos/fifo.c",
    "../src/avrtos/flags.c",
    "../src/avrtos/idle.c",
    "../src/avrtos/init.c",
    "../src/avrtos/kernel.c",
    "../src/avrtos/mem_slab.c",
    "../src/avrtos/msgq.c",
    "../src/avrtos/mutex.c",
    "../src/avrtos/prng.c",
    "../src/avrtos/ring.c",
    "../src/avrtos/semaphore.c",
    "../src/avrtos/signal.c",
    "../src/avrtos/stack_sentinel.c",
    "../src/avrtos/stdout.c",
    "../src/avrtos/sysclock.c",
    "../src/avrtos/timer.c",
    "../src/avrtos/workqueue.c",

    "../src/avrtos/rust_helpers.c",
];

const HEADERS: &[&str] = &[
    "../src/avrtos/devices/tcn75.h",
    "../src/avrtos/devices/mcp2515.h",
    "../src/avrtos/devices/mcp2515_priv.h",

    "../src/avrtos/drivers.h",
    "../src/avrtos/drivers/exti.h",
    "../src/avrtos/drivers/gpio.h",
    "../src/avrtos/drivers/i2c.h",
    "../src/avrtos/drivers/i2c_defs.h",
    "../src/avrtos/drivers/spi.h",
    "../src/avrtos/drivers/timer.h",
    "../src/avrtos/drivers/timer_defs.h",
    "../src/avrtos/drivers/usart.h",

    "../src/avrtos/dstruct/debug.h",
    "../src/avrtos/dstruct/dlist.h",
    "../src/avrtos/dstruct/slist.h",
    "../src/avrtos/dstruct/tqueue.h",
    "../src/avrtos/dstruct/tdqueue.h",
    
    "../src/avrtos/misc/led.h",
    "../src/avrtos/misc/serial.h",

    "../src/avrtos.h",
    "../src/avrtos/assert.h",
    "../src/avrtos/atomic.h",
    "../src/avrtos/avrtos.h",
    "../src/avrtos/avrtos_arduinoide_conf.h",
    "../src/avrtos/avrtos_conf.h",
    "../src/avrtos/canaries.h",
    "../src/avrtos/defines.h",
    "../src/avrtos/debug.h",
    "../src/avrtos/errno.h",
    "../src/avrtos/event.h",
    "../src/avrtos/fault.h",
    "../src/avrtos/fifo.h",
    "../src/avrtos/flags.h",
    "../src/avrtos/idle.h",
    "../src/avrtos/init.h",
    "../src/avrtos/kernel.h",
    "../src/avrtos/kernel_private.h",
    "../src/avrtos/logging.h",
    "../src/avrtos/mem_slab.h",
    "../src/avrtos/msgq.h",
    "../src/avrtos/mutex.h",
    "../src/avrtos/prng.h",
    "../src/avrtos/ring.h",
    "../src/avrtos/semaphore.h",
    "../src/avrtos/signal.h",
    "../src/avrtos/stack_sentinel.h",
    "../src/avrtos/io.h",
    "../src/avrtos/sys.h",
    "../src/avrtos/sysclock.h",
    "../src/avrtos/timer.h",
    "../src/avrtos/types.h",
    "../src/avrtos/workqueue.h",

    "../src/avrtos/rust_helpers.h",
];

const INCLUDES: &[&str] = &[
    "../src/",
    "../src/avrtos",
    "../src/avrtos/arch",
    "../src/avrtos/devices",
    "../src/avrtos/drivers",
    "../src/avrtos/dstruct",
    "../src/avrtos/misc",
];

const FEATURE_KERNEL_DEBUG: bool = false;

// TODO reset to default value (1000us)
const FEATURE_KERNEL_SYSCLOCK_PERIOD_US: u32 = 1000;

fn compile_avrtos_sources() {
    let mut build = cc::Build::new();

    let linker_script;
    #[cfg(feature = "avrtos-linker-script")]
    {
        linker_script = "1";
        println!("cargo:rustc-link-arg=-T{}", LINKER_SCRIPT);
    }

    #[cfg(not(feature = "avrtos-linker-script"))]
    {
        linker_script = "0";
    }

    build
        .compiler("avr-gcc")
        .files(SOURCES.iter())
        .includes(INCLUDES.iter())
        .static_flag(true)
        .debug(true)
        .no_default_flags(true)
        // mcu
        .flag("-mmcu=atmega2560")
        // standard flags
        .flag("-Wall")
        .flag("-fno-fat-lto-objects")
        .flag("-funsigned-char")
        .flag("-funsigned-bitfields")
        .flag("-fpack-struct")
        .flag("-fshort-enums")
        .flag("-fdata-sections")
        .flag("-ffunction-sections")
        .flag("-fno-split-wide-types")
        .flag("-fno-tree-scev-cprop")
        .flag("-flto")
        .flag("-Wstrict-prototypes")
        .flag("-std=gnu11")

        // debug infos
        .flag("-g")
        // .flag("-Og")
        // .flag("-gdwarf-3")
        // .flag("-gstrict-dwarf")
        .flag("-Os")

        

        // disable strict warnings
        .flag("-Wno-expansion-to-defined")
        .flag("-Wno-implicit-fallthrough")
        .flag("-Wno-sign-compare")
        .flag("-Wno-type-limits")

        // target config flags
        .flag("-DF_CPU=16000000UL")

        // Default config (must match what is in the bindgen script avrtos-sys/scripts/gen.sh at any cost)
        .flag("-DCONFIG_RUST=1")
        .flag("-DZ_CONFIG_ARDUINO_FRAMEWORK=0")
        .flag("-DZ_CONFIG_PLATFORMIO_IDE=0")
        .flag(format!("-DCONFIG_AVRTOS_LINKER_SCRIPT={}", linker_script))
        .flag("-DCONFIG_THREAD_MAIN_COOPERATIVE=1")
        .flag("-DCONFIG_INTERRUPT_POLICY=1")
        .flag("-DCONFIG_THREAD_EXPLICIT_MAIN_STACK=0")
        .flag("-DCONFIG_THREAD_MAIN_STACK_SIZE=0x200")
        .flag("-DCONFIG_USE_STDLIB_HEAP_MALLOC_MAIN=0")
        .flag("-DCONFIG_USE_STDLIB_HEAP_MALLOC_THREAD=0")
        .flag("-DCONFIG_THREAD_DEFAULT_SREG=128")
        .flag("-DCONFIG_KERNEL_THREAD_TERMINATION_TYPE=0")
        .flag("-DCONFIG_KERNEL_SCHEDULER_COMPARE_THREADS_BEFORE_SWITCH=1")
        .flag("-DCONFIG_KERNEL_COOPERATIVE_THREADS=1")
        .flag(format!("-DCONFIG_KERNEL_SYSCLOCK_PERIOD_US={}llu", FEATURE_KERNEL_SYSCLOCK_PERIOD_US))
        .flag(format!("-DCONFIG_KERNEL_TIME_SLICE_US={}llu", FEATURE_KERNEL_SYSCLOCK_PERIOD_US))
        .flag("-DCONFIG_KERNEL_SYSLOCK_HW_TIMER=1")
        .flag("-DCONFIG_CONFIG_KERNEL_TICKS_COUNTER_40BITS=1")
        .flag("-DCONFIG_KERNEL_DELAY_OBJECT_U32=1")
        .flag("-DCONFIG_KERNEL_AUTO_INIT=0")
        .flag("-DCONFIG_KERNEL_MINICORE_SAVE_RESET_CAUSE=0")
        .flag("-DCONFIG_KERNEL_CLEAR_WDT_ON_INIT=0")
        .flag("-DCONFIG_KERNEL_THREAD_IDLE=1")
        .flag("-DCONFIG_KERNEL_THREAD_IDLE_ADD_STACK=0x50")
        .flag("-DCONFIG_THREAD_IDLE_COOPERATIVE=0")
        .flag("-DCONFIG_IDLE_HOOK=0")
        .flag("-DCONFIG_THREAD_CANARIES=0")
        .flag("-DCONFIG_THREAD_CANARIES_SYMBOL=0xAA")
        .flag("-DCONFIG_THREAD_STACK_SENTINEL=0")
        .flag("-DCONFIG_THREAD_STACK_SENTINEL_SIZE=1")
        .flag("-DCONFIG_THREAD_STACK_SENTINEL_SYMBOL=0x55")
        .flag("-DCONFIG_THREAD_MONITOR=0")
        .flag("-DCONFIG_THREAD_MAIN_MONITOR=0")
        .flag("-DCONFIG_SYSTEM_WORKQUEUE_ENABLE=0")
        .flag("-DCONFIG_SYSTEM_WORKQUEUE_STACK_SIZE=0x200")
        .flag("-DCONFIG_SYSTEM_WORKQUEUE_COOPERATIVE=0")
        .flag("-DCONFIG_WORKQUEUE_DELAYABLE=0")
        .flag("-DCONFIG_KERNEL_ASSERT=0")
        .flag("-DCONFIG_KERNEL_ARGS_CHECKS=0")
        .flag("-DCONFIG_KERNEL_TIMERS=0")
        .flag("-DCONFIG_KERNEL_EVENTS=0")
        .flag("-DCONFIG_KERNEL_EVENTS_ALLOW_NO_WAIT=1")
        .flag("-DCONFIG_SERIAL_AUTO_INIT=0u")
        .flag("-DCONFIG_SERIAL_USART_BAUDRATE=115200u")
        .flag("-DCONFIG_STDIO_PRINTF_TO_USART=-1")
        .flag("-DCONFIG_LOGGING_SUBSYSTEM=1")
        .flag("-DCONFIG_KERNEL_UPTIME=0")
        .flag("-DCONFIG_KERNEL_TIME_API=0")
        .flag("-DCONFIG_KERNEL_TIME_API_MS_PRECISION=0")
        .flag("-DCONFIG_KERNEL_ATOMIC_API=1")
        .flag("-DCONFIG_KERNEL_DEBUG_PREEMPT_UART=0")
        .flag("-DCONFIG_KERNEL_API_NOINLINE=0")
        .flag(format!("-DCONFIG_KERNEL_SYSCLOCK_DEBUG={}", FEATURE_KERNEL_DEBUG as u8))
        .flag(format!("-DCONFIG_KERNEL_SCHEDULER_DEBUG={}", FEATURE_KERNEL_DEBUG as u8))
        .flag("-DCONFIG_FD_MAX_COUNT=0")
        .flag("-DCONFIG_KERNEL_REENTRANCY=0")
        .flag("-DCONFIG_DRIVERS_USART0_ASYNC=0")
        .flag("-DCONFIG_DRIVERS_USART1_ASYNC=0")
        .flag("-DCONFIG_DRIVERS_USART2_ASYNC=0")
        .flag("-DCONFIG_DRIVERS_USART3_ASYNC=0")
        .flag("-DCONFIG_DRIVERS_TIMER0_API=0")
        .flag("-DCONFIG_DRIVERS_TIMER1_API=0")
        .flag("-DCONFIG_DRIVERS_TIMER2_API=0")
        .flag("-DCONFIG_DRIVERS_TIMER3_API=0")
        .flag("-DCONFIG_DRIVERS_TIMER4_API=0")
        .flag("-DCONFIG_DRIVERS_TIMER5_API=0")
        .flag("-DCONFIG_KERNEL_SYSTICK_GPIOB_DEBUG=0")
        .flag("-DCONFIG_KERNEL_STATS=0")
        .flag("-DCONFIG_KERNEL_FAULT_VERBOSITY=1")
        .flag("-DCONFIG_KERNEL_DEBUG_GPIO=0")
        .flag("-DCONFIG_KERNEL_DEBUG_GPIO_SYSTICK=1")
        .flag("-DCONFIG_SPI_ASYNC=0")
        .flag("-DCONFIG_AVRTOS_BANNER_ENABLE=0")
        .flag("-DCONFIG_AVRTOS_BANNER=\"*** AVRTOS RUST ***\\n\"")
        .flag("-DCONFIG_I2C_DRIVER_ENABLE=1")
        .flag("-DCONFIG_I2C0_ENABLED=1")
        .flag("-DCONFIG_I2C1_ENABLED=0")
        .flag("-DCONFIG_I2C_INTERRUPT_DRIVEN=0")
        .flag("-DCONFIG_I2C_FREQ=400000")
        .flag("-DCONFIG_I2C_BLOCKING=1")
        .flag("-DCONFIG_I2C_MAX_BUF_LEN_BITS=3u")
        .flag("-DCONFIG_I2C_LAST_ERROR=1")
        .flag("-DCONFIG_I2C_DEBUG")
        .flag("-DCONFIG_KERNEL_FLAGS_SIZE=1")
        .flag("-DCONFIG_DEVICE_MCP2515=0");

    // build
    //     .flag("-Wl,--gc-sections")
    //     .flag("-Wl,-Map,map.out");

    #[cfg(feature = "qemu")]
    {
        build.flag("-D__QEMU__");
    }

    build.ar_flag("v");

    build.compile("avrtos");

    for source in SOURCES {
        println!("cargo:rerun-if-changed={}", source);
    }

    for header in HEADERS {
        println!("cargo:rerun-if-changed={}", header);
    }

    // add link args
    // println!("cargo:rustc-flag=-Wl,--gc-sections");
    // println!("cargo:rustc-flag=-Wl,-Map,map.out");
    // println!("link-arg=-Wl,--verbose");

    println!("cargo:rustc-link-lib=static=avrtos");
}

fn main() {
    compile_avrtos_sources()
}
