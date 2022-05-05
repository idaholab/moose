#!/usr/bin/env python3
import os
import sys
import re
import yaml

def get_registered_apps(parts_list):
    """
    return a dictionary of registered apps (including our own)
    """
    parts = [f'{x}APP' for x in re.findall(r'-D(\w+)_ENABLED',
                                           ''.join(parts_list))]
    app_name = re.sub('_|-', '', f'{os.path.basename(parts_list[0])}')
    print(app_name)
    app_and_test = set([f'{app_name.upper()}APP',
                         f'{app_name.upper().replace("TEST", "")}TESTAPP'])
    parts.extend(app_and_test)
    return {'registered_apps': parts }

def getParts(parts_list):
    """
    loop through desired attributes to be recorded
    into the resource file, return the resulting dictionary
    """
    attributes = ['get_registered_apps']
    growing = {}
    for attribute in attributes:
        growing = {**growing | getattr(sys.modules[__name__],
                                       attribute)(parts_list)}
    return growing

def main(args):
    file_name = f'.{os.path.basename(args[1])}'
    file_path = os.path.dirname(args[1])
    yaml_contents = getParts(args[1:])
    with open(os.path.join(file_path, file_name), 'w') as stream:
        yaml.dump(yaml_contents, stream, default_flow_style=False)

if __name__ == '__main__':
    sys.exit(main(sys.argv))
