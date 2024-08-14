#![no_std]
#![no_main]
#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
#![allow(dead_code)]

use core::ffi::c_char;

use avrtos_sys::{
    k_sleep, k_timeout_t, led_init, led_off, led_on, led_toggle, my_yield, serial_hex16, serial_init_baud, serial_print, sleep, sleep_1s, z_avrtos_init, z_rust_sys, z_yield
};
use panic_halt as _;

const boot: &'static str = "avrtos starting\n\x00";
const msg: &'static str = "\n\x00";

#[arduino_hal::entry]
fn main() -> ! {
    unsafe {
        z_avrtos_init();
        serial_init_baud(115200);
        serial_print(boot.as_ptr() as *const c_char);
        led_init();
    }

    sleep_1s();
    unsafe {
        led_toggle();
    }
    sleep_1s();
    unsafe {
        led_toggle();
    }
    sleep_1s();
    unsafe {
        led_toggle();
    }
    sleep_1s();
    unsafe {
        led_toggle();
    }
    sleep_1s();
    unsafe {
        led_toggle();
    }
    sleep_1s();
    unsafe {
        led_toggle();
    }
    sleep_1s();
    unsafe {
        led_toggle();
    }
    
    loop {
    }

}
