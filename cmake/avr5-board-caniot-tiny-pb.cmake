#
# Copyright (c) 2022 Lucas Dietrich <ld.adecy@gmail.com>
#
# SPDX-License-Identifier: Apache-2.0
#

# Ignore this toolchain file as it is specific to the following personal project:
# https://github.com/lucasdietrich/caniot-device

set(F_CPU 16000000UL)
set(MCU atmega328p)
set(LINKER_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/../architecture/avr/avrtos-avr5-atmega328p.xn)
set(QEMU_MCU uno)
set(PROG_TYPE arduino)
set(PROG_PARTNO m328pb)

include(${CMAKE_CURRENT_LIST_DIR}/avr.cmake)