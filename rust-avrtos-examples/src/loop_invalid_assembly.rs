#![no_std]
#![no_main]
#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
#![allow(dead_code)]
#![feature(core_intrinsics)]

use core::{ffi::c_char, intrinsics::unreachable};

use avrtos_sys::{
    serial_init_baud, serial_print
};
use panic_halt as _;

const boot: &'static str = "avrtos starting\n\x00";

#[arduino_hal::entry]
fn main() -> ! {
    unsafe {
        serial_init_baud(115200);
        serial_print(boot.as_ptr() as *const c_char);
    }

    loop {
    }
}
