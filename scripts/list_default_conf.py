import re

def extract_defined_constants(file_path):
    constants = {}
    define_pattern = re.compile(r'#define\s+(\S+)\s+(.+)')
    
    with open(file_path, 'r', encoding='utf-8') as file:
        for line in file:
            match = define_pattern.match(line.strip())
            if match:
                constant_name, constant_value = match.groups()
                constants[constant_name] = constant_value
    
    return constants
        
if __name__ == "__main__":
    conf_path = "src/avrtos/avrtos_conf.h"
    constants = extract_defined_constants(conf_path)
    
    for constant_name, constant_value in constants.items():
        print(f"{constant_name} = {constant_value}")