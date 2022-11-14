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
import platform
import json
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
        tell(args.quiet, f'{"fatal" if args.influential else "warning"}: commit {args.commit}'
                          ' is not a valid git commit')
        tell(False, 'arbitrary', 1 if args.influential else 0)
    return out

def parse_args(args):
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
    parser.add_argument('--oras', action='store_true', default=False,
                        help='Output in oras/harbor format')
    parser.add_argument('--conda', action='store_true', default=False,
                        help='Output in Conda naming convention format')
    parser.add_argument('--json', action='store_true', default=False,
                        help='Output in JSON format (itemized information)')
    parser.add_argument('--yaml', action='store_true', default=False,
                        help='Output in YAML format (itemized information)')
    parser.add_argument('-d', '--dependencies', action='store_true', default=False,
                        help='List dependencies for said library in oras format')

    # Default to moose if library supplied matches nothing
    _supply_default = False
    if len(args[:-1]):
        if args[1] not in ENTITIES and args[1][0] != '-':
            _supply_default = True
            sys.argv[1] = 'moose'
    args = parser.parse_args()
    if args.json and args.yaml:
        print('cannot output both json and yaml simultaneously')
        sys.exit(1)
    if args.oras and args.conda:
        print('cannot output both conda and oras simultaneously')
        sys.exit(1)
    # When supplying a non-tracked library and we are not asking for a dependency list then instead
    # print the libmesh hash (we will never supply a pre-built moose that will work to build other
    # applications). If we are asking for a list, then proceed to use 'moose' due to the dependency
    # routines will not print itself as a dependency (we will get libmesh)
    if _supply_default and not args.dependencies:
        args.library = 'libmesh'
    if _supply_default and not args.quiet:
        print('Untracked library, using "moose" as your dependency')
    return args

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

def parse_meta(args):
    """ populate and return dictionary making up the contents involved with generating hashes """
    _dtmp = read_yamlfile(args, f'./{os.path.relpath(YAML_FILE)}')
    _meta = {}
    for group_entity in ENTITIES:
        if group_entity in _dtmp['packages']:
            _meta[group_entity] = _meta.get('group_entity', {'NAME'        : [],
                                                             'VERSION'     : [],
                                                             'BUILD'       : [],
                                                             'INFLUENTIAL' : [],
                                                             'HASH'        : []})
            _hash_list = []
            _group = _meta[group_entity]
            for entity in _dtmp['packages'][group_entity]:
                if entity.find('meta.yaml') != -1:
                    (name, version, build) = get_tag(args, entity)
                    _group['NAME'].append(name)
                    _group['VERSION'].append(version)
                    _group['BUILD'].append(build)
                _group['INFLUENTIAL'].append(entity)
                _hash_list.append(git_hash(args, entity))
            _group['HASH'].append(get_hash(_hash_list))
    return _meta

def get_tag(args, meta_yaml):
    """ read conda-build jinja2 styled template, and return a name, version, build tuple """
    out = read_yamlfile(args, meta_yaml)
    return (out['package']['name'],
            out['package']['version'],
            out['build']['string'])

def get_hash(hash_list):
    """ return a seven character hash for given list of strings """
    return hashlib.md5(''.join(hash_list).encode('utf-8')).hexdigest()[:7]

# With so many ways to print output, suppress the to-many-branches > 12 pylint rating
# pylint: disable=too-many-branches
def print_output(args, meta):
    """ format and print the output based on supplied arguments """
    formated = []
    jq_formated = {}
    separator = f'{" " if args.yaml else "="}'
    # Prefix packages with moose- when needed (we don't have a package called moose-moose)
    moose_prefix = f'{"moose-" + args.library if args.library != "moose" else "moose"}'
    if args.influential:
        for zip_key in ENTITIES:
            formated.extend(meta[zip_key]['INFLUENTIAL'])
            jq_formated[zip_key] = meta[zip_key]['INFLUENTIAL']
            if zip_key == args.library:
                break
    elif args.dependencies:
        for zip_key in ENTITIES:
            if zip_key == args.library:
                break
            if args.conda:
                name = meta[zip_key]['NAME'][0]
                version = meta[zip_key]['VERSION'][0]
                build = meta[zip_key]['BUILD'][0]
                formated.append(f'{name}{separator}{version}{separator}{build}')
                jq_formated[args.library] = jq_formated.get(args.library, [])
                jq_formated[args.library].append(f'{name}{separator}{version}{separator}{build}')
            elif args.oras:
                uri = f'moose-{zip_key}/{zip_key}-{platform.machine()}:{meta[zip_key]["HASH"][0]}'
                formated.append(uri)
                jq_formated[args.library] = jq_formated.get(args.library, [])
                jq_formated[args.library].append(uri)
            else:
                formated.append(meta[zip_key]["HASH"][0])
                jq_formated[args.library] = jq_formated.get(args.library, [])
                jq_formated[args.library].append(meta[zip_key]["HASH"][0])
        formated.reverse()
        jq_formated[args.library].reverse()

    elif args.conda and args.library in ENTITIES:
        version = meta[args.library]['VERSION'][0]
        build = meta[args.library]['BUILD'][0]
        formated.append(f'{moose_prefix}={version}={build}')
        jq_formated[args.library] = {'NAME'    : moose_prefix,
                                     'VERSION' : version,
                                     'BUILD'   : build}
    elif args.oras:
        uri = f'{moose_prefix}/{args.library}-{platform.machine()}:{meta[args.library]["HASH"][0]}'
        formated.append(uri)
    else:
        _hash = meta[args.library]['HASH'][0]
        formated.append(_hash)
        jq_formated[args.library] = {'HASH' : _hash }
    if args.json:
        print(json.dumps(jq_formated))
    elif args.yaml:
        print(yaml.dump(jq_formated))
    else:
        print('\n'.join(formated))

def main():
    """ print hash for supplied library """
    args = parse_args(sys.argv)
    meta = parse_meta(args)
    print_output(args, meta)

if __name__ == '__main__':
    main()
