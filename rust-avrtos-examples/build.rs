const LINKER_SCRIPT: &'static str = "architecture/avr/avrtos-avr6.xn";

fn main() {
    println!("cargo:rustc-link-arg=-T{}", LINKER_SCRIPT);
    println!("cargo:rustc-link-lib=static=avrtos");
}
