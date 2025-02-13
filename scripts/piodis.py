# require avr-gcc toolchain

import os
import os.path
import subprocess

def dis(elf: str, elf_dir: str):
    commands = [
        (f"avr-objdump -S {elf}", f"{elf_dir}/objdump_src.s"),
        (f"avr-objdump -d {elf}", f"{elf_dir}/objdump.s"),
        (f"avr-objdump -D {elf}", f"{elf_dir}/objdump_all.s"),
        (f"avr-objdump -h {elf}", f"{elf_dir}/objdump_sections.s"),
        (f"avr-readelf -a {elf}", f"{elf_dir}/readelf.txt"),
        (f"avr-readelf -x .data {elf}", f"{elf_dir}/section_data.txt"),
        (f"avr-readelf -x .bss {elf}", f"{elf_dir}/section_bss.txt"),
        (f"avr-readelf -x .noinit {elf}", f"{elf_dir}/section_noinit.txt"),
        (f"avr-readelf -x .text {elf}", f"{elf_dir}/section_text.txt"),
        (f"avr-nm --print-size --size-sort --radix=x {elf}", f"{elf_dir}/nm-size.txt"),
        (f"avr-nm {elf}", f"{elf_dir}/nm.txt"),
        (f"avr-size {elf}", f"{elf_dir}/size.txt"),
    ]

    for command in commands:
        process = subprocess.Popen(command[0].split(), stdout=open(command[1], "w+"), cwd=".")
        output, error = process.communicate()

        if output or error:
            print(output, error)

    return commands

if __name__ == "__main__":
    DIR = "./.pio/build/"

    # list projects
    for name in os.listdir(DIR):
        project_dir = os.path.join(DIR, name)
        if os.path.isfile(project_dir):
            continue
        elf_files = [file for file in os.listdir(project_dir) if file.endswith('.elf')]
        if elf_files:  # Check if any .elf file was found
            elf = os.path.join(project_dir, elf_files[0])  # Get the first .elf file
            if os.path.isfile(elf):
                commands = dis(name, elf, project_dir)
                print(f"- Disassembled '{name}' :\n\t" + '\n\t'.join(map(lambda cmd: cmd[1], commands)))
                