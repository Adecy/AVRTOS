#![no_std]
#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
#![allow(dead_code)]

mod bindings;

pub use bindings::*;

pub fn rust_sleep() {
    // Doing this is highly BROKEN and I don't know why !!!
    let timeout = k_timeout_t { value: 1000 };
    unsafe { k_sleep(timeout)}
}

pub fn rust_yield() {
    unsafe {
        let key = z_rust_irq_lock();
        z_yield();
        z_rust_irq_unlock(key);
    }
}
