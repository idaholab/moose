#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

"""
Generate hash based on vetted versions of libraries as listed in
module_hash.yaml's (zip_keys)
"""

import os
import sys
import argparse
import hashlib
import subprocess
import yaml
from jinja2 import Environment, DictLoader

MOOSE_DIR = os.environ.get('MOOSE_DIR',
                           os.path.abspath(os.path.join(os.path.dirname(
                               os.path.realpath(__file__)), '..')))

# Load resource file so argparse 'choices' can function properly
YAML_FILE = os.path.abspath(os.path.join(MOOSE_DIR, "scripts", "module_hash.yaml"))

try:
    with open(f'{YAML_FILE}', 'r', encoding='utf-8') as rc_file:
        ENTITIES = yaml.safe_load(rc_file)['zip_keys']

except FileNotFoundError:
    print(f'fatal: {YAML_FILE} not found')
    sys.exit(1)

except yaml.scanner.ScannerError:
    print(f'fatal: {YAML_FILE} parsing error')
    sys.exit(1)

def tell(quiet, message, and_exit=None):
    """ print a message to stdout and conditionally exit with code """
    if not quiet:
        print(message)
    if and_exit is not None:
        sys.exit(and_exit)

def run_subprocess(args, command):
    """ run a command return output """
    try:
        out = subprocess.check_output(command, encoding='utf-8', stderr=subprocess.DEVNULL)
    # commit was not found in git
    except subprocess.CalledProcessError:
        tell(args.quiet, f'warning: {args.commit} is not a valid git commit')
        tell(False, 'arbitrary', 0)
    return out

def parse_args():
    """ parse arguments """
    parser = argparse.ArgumentParser(description='Supplies a hash for a given library')
    parser.add_argument('library', metavar='library', choices=ENTITIES,
                        help=f'choose from: {", ".join(ENTITIES)}')
    parser.add_argument('commit', nargs='?', metavar='commit', default='HEAD',
                        help='default %(default)s')
    parser.add_argument('-q', '--quiet', action='store_true', default=False,
                        help='Do not print warnings')
    parser.add_argument('-i', '--influential', action='store_true', default=False,
                        help='List influential files involved with hash generation '
                        'then exit')
    parser.add_argument('-t', '--tag', action='store_true', default=False,
                        help='Mimic Conda and print a <VERSION>_<BUILD> string')
    return parser.parse_args()

def git_file(args, entity):
    """ use git to return file at commit """
    command = ['git', 'show', f'{args.commit}:{os.path.join(MOOSE_DIR, entity)}' ]
    return run_subprocess(args, command)

def git_hash(args, entity):
    """ use git to return hash at commit """
    command = ['git', 'ls-tree', args.commit, os.path.join(MOOSE_DIR, entity)]
    out = run_subprocess(args, command)
    try:
        _stmp = out.split()[2]
    # file does not exist in repo (git does not error for this scenario)
    except IndexError:
        tell(args.quiet, f'warning: {entity} does not exist')
        tell(False, 'arbitrary', 0)
    return _stmp

def parse_jinja(jinja_template):
    """ read jinja_template and return proper yaml (harmless if not jinja) """
    # Handle pin_subpackage specific to 'conda-build' templates
    # pylint: disable=unused-argument
    def pin_subpackage(package, max_pin):
        return
    config = {'pin_subpackage' : pin_subpackage }
    env = Environment(loader = DictLoader({'' : jinja_template }),
                                           trim_blocks=True,
                                           lstrip_blocks=True)
    meta_template = env.get_template('')
    meta_render = meta_template.render(config)
    return meta_render

def read_yamlfile(args, yaml_file):
    """ read yaml_file file at specified commit """
    _meta = {}
    command = ['git', 'show', f'{args.commit}:{yaml_file}']
    str_out = run_subprocess(args, command)
    yaml_out = parse_jinja(str_out)
    try:
        _meta = yaml.safe_load(yaml_out)
    except yaml.YAMLError:
        # Resource file does not exist at specified commit
        message = f'There was an error parsing yaml file: {yaml_file}'
        if args.influential:
            tell(False, f'fatal: {message}', 1)
        tell(args.quiet, f'warning: {message}')
    return _meta

def build_list(args):
    """ return list of files associated with supplied library """
    _dtmp = read_yamlfile(args, f'./{os.path.relpath(YAML_FILE)}')
    _ltmp = []
    for group_entity in ENTITIES:
        if group_entity in _dtmp['packages']:
            if args.tag and group_entity == args.library:
                for entity in _dtmp['packages'][group_entity]:
                    if args.tag and entity.find('meta.yaml') == -1:
                        continue
                    _ltmp.append(entity)
            elif not args.tag:
                for entity in _dtmp['packages'][group_entity]:
                    _ltmp.append(entity)
        if group_entity == args.library:
            break
    return _ltmp

# Disable pylint to support supplemental feature not generally used
# pylint: disable=import-outside-toplevel
def get_tag(args, meta_yaml):
    """ read conda-build jinja2 styled template, and return a version_build string """
    out = read_yamlfile(args, meta_yaml)
    return f'{out["package"]["version"]}_{out["build"]["string"]}'

def main():
    """ print hash for supplied library """
    args = parse_args()
    hash_tree = []
    entity_list = []
    for an_entity in build_list(args):
        entity_list.append(os.path.abspath(os.path.join(MOOSE_DIR, an_entity)))
        hash_tree.append(git_hash(args, an_entity))
    if args.influential:
        print('\n'.join(entity_list))
    else:
        if args.tag:
            print(get_tag(args, os.path.relpath(entity_list[0])))
        else:
            print(hashlib.md5(''.join(hash_tree).encode('utf-8')).hexdigest()[:7])

if __name__ == '__main__':
    main()
