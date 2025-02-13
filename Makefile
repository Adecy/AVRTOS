# GENERATOR?="Unix Makefiles"
# GENERATOR_COMMAND?="make"
# GENERATOR_ARGS?="--no-print-directory"

GENERATOR?="Ninja"
GENERATOR_COMMAND?="ninja"
GENERATOR_ARGS?=

DEVICE?=/dev/ttyACM0
SAMPLE?=shell
TOOLCHAIN_FILE?=cmake/avr6-atmega2560.cmake
BAUDRATE?=115200
QEMU?=OFF

# if QEMU is enabled, set CMAKE_BUILD_TYPE to Debug by default
ifeq ($(QEMU),ON)
	CMAKE_BUILD_TYPE?=Debug
else
	CMAKE_BUILD_TYPE?=Release
endif

all: single

.PHONY: single cmake multiple upload monitor qemu run_qemu format clean piogen arduino_gen gen flash rust rust_build rust_bindgen

cmake:
	cmake -S . -B build \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		-DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN_FILE) \
		-DCMAKE_GENERATOR=$(GENERATOR) \
		-DPROG_DEV=$(DEVICE) \
		-DBAUDRATE=$(BAUDRATE) \
		-DQEMU=$(QEMU) \
		-DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE)

multiple:
	cmake -S . -B build \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		-DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN_FILE) \
		-DCMAKE_GENERATOR=$(GENERATOR) \
		-DPROG_DEV=$(DEVICE) \
		-DBAUDRATE=$(BAUDRATE) \
		-DQEMU=$(QEMU) \
		-DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE)
	$(GENERATOR_COMMAND) -C build $(GENERATOR_ARGS)

single:
	cmake -S . -B build \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		-DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN_FILE) \
		-DCMAKE_GENERATOR=$(GENERATOR) \
		-DENABLE_SINGLE_SAMPLE=$(SAMPLE) \
		-DPROG_DEV=$(DEVICE) \
		-DBAUDRATE=$(BAUDRATE) \
		-DQEMU=$(QEMU) \
		-DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE)
	$(GENERATOR_COMMAND) -C build $(GENERATOR_ARGS)

drv-timer:
	cmake -S . -B build \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		-DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN_FILE) \
		-DCMAKE_GENERATOR=$(GENERATOR) \
		-DENABLE_SINGLE_SAMPLE=$@ \
		-DPROG_DEV=$(DEVICE) \
		-DBAUDRATE=$(BAUDRATE) \
		-DQEMU=ON \
		-DCMAKE_BUILD_TYPE=Debug
	$(GENERATOR_COMMAND) -C build $(GENERATOR_ARGS)

drv-i2c-tcn75:
	cmake -S . -B build \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		-DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN_FILE) \
		-DCMAKE_GENERATOR=$(GENERATOR) \
		-DENABLE_SINGLE_SAMPLE=$@ \
		-DPROG_DEV=$(DEVICE) \
		-DBAUDRATE=$(BAUDRATE) \
		-DQEMU=OFF \
		-DCMAKE_BUILD_TYPE=Release
	$(GENERATOR_COMMAND) -C build $(GENERATOR_ARGS)

upload:
	$(GENERATOR_COMMAND) -C build upload $(GENERATOR_ARGS)

flash: upload

monitor:
	$(GENERATOR_COMMAND) -C build monitor $(GENERATOR_ARGS)

qemu:
	$(GENERATOR_COMMAND) -C build qemu $(GENERATOR_ARGS)

debug: qemu

run_qemu:
	$(GENERATOR_COMMAND) -C build run_qemu $(GENERATOR_ARGS)

format:
	find src -iname *.h -o -iname *.c -o -iname *.cpp | xargs clang-format -i
	find examples -iname *.h -o -iname *.c -o -iname *.cpp -o -iname *.ino | xargs clang-format -i

clean:
	rm -rf build
	rm -rf target

piogen:
	python3 ./scripts/piogen.py

arduino_gen:
	python3 ./scripts/arduinogen.py

arduino_lint:
	arduino-lint --compliance strict --library-manager update
	
metrics:
	scripts/metrics-build-examples-default.sh

# Prepare for release
publish: piogen arduino_gen format clean multiple metrics arduino_lint

rust_bindgen:
	./rust-avrtos-sys/scripts/gen.sh
	
rust_build:
	cargo build --package rust-avrtos-examples --release --bin loop_invalid_assembly
	python3 scripts/rustdis.py

rust:
	cargo run --package rust-avrtos-examples --release --bin loop_invalid_assembly