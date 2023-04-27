#!/usr/bin/env python3
""" helper script to write a meta resource file containing useful build time attributes """
import os
import sys
import re
import yaml
from setuptools import distutils  # for bool conversions

def get_registered_apps(parts_list):
    """
    return a dictionary of registered apps (including our own)
    """
    parts = [f'{x}APP' for x in re.findall(r'-D(\w+)_ENABLED',
                                           ''.join(parts_list))]
    app_name = re.sub('_|-', '', f'{os.path.basename(parts_list[0])}')
    app_and_test = set([f'{app_name.upper()}APP',
                         f'{app_name.upper().replace("TEST", "")}TESTAPP'])
    parts.extend(app_and_test)
    return { 'registered_apps': parts }

def parse_keyvalues(parts_list):
    """
    return a dictionary of key=value pairs
    """
    strbool = distutils.util.strtobool
    parts = re.findall(r'(\w+)=(\w+)', ' '.join(parts_list))
    _tmp_dict = {}
    for key_item, item_value in parts:
        # generalize bools to True, False
        try:
            _value = strbool(item_value) == 1
        except ValueError:
            _value = item_value
        _tmp_dict[key_item] = _value
    return _tmp_dict

def get_parts(parts_list):
    """
    loop through desired attributes to be recorded
    into the resource file, return the resulting dictionary
    """
    parse_methods = ['get_registered_apps',
                     'parse_keyvalues']
    growing = {}
    for method in parse_methods:
        growing = {**growing | getattr(sys.modules[__name__],
                                       method)(parts_list)}
    return growing

def main(args):
    """
    parse incoming args, then update file.
    """
    file_name = f'.{os.path.basename(args[1])}'
    file_path = os.path.dirname(args[1])
    full_path = os.path.join(file_path, file_name)
    new_parts = get_parts(args[1:])
    file_attribs = 'w+'
    if os.path.exists(full_path):
        file_attribs = 'r+'
    with open(full_path, file_attribs, encoding='utf-8') as contents:
        try:
            _incoming = yaml.safe_load(contents)
            if isinstance(_incoming, dict):
                _incoming.update(new_parts)
            else:
                _incoming = new_parts
        # We will overwrite on parse errors
        except yaml.parser.ParserError:
            _incoming = new_parts
        contents.seek(0)
        contents.truncate()
        yaml.dump(_incoming, contents, default_flow_style=False)

if __name__ == '__main__':
    sys.exit(main(sys.argv))
