from piodis import dis

import os

if __name__ == "__main__":
    for exe in ["loop_invalid_assembly", "serial_demo", "sleep_demo"]:
        for build_type in ["release", "debug"]:
            rust_example = f"target/avr-atmega2560/{build_type}/{exe}.elf"
            if os.path.isfile(rust_example):
                commands = dis(rust_example, f"target/avr-atmega2560/{build_type}")
                print(f"- Disassembled '{exe}' ({build_type}) :\n\t" + '\n\t'.join(map(lambda cmd: cmd[1], commands)))
