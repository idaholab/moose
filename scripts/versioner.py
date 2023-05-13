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
import re
import subprocess
import platform
import json
from collections import OrderedDict
import yaml
import jinja2

MOOSE_DIR = os.environ.get('MOOSE_DIR',
                           os.path.abspath(os.path.join(os.path.dirname(
                               os.path.realpath(__file__)), '..')))

### Tracking Libraries
# note: Order is important only for historical lookups; git_ancestor(commit) == True
TRACKING_LIBRARIES = ['mpich', 'petsc', 'libmesh', 'wasp', 'moose', 'app']

### Beautify the output of jinja2 rendered content that may only exists in conda-build scenarios
# pylint: disable=unused-argument
def undefined(arg, *args, **kwargs):
    """
    Handle any number of passed arguments.
    The *, ** args is needed by pin_subpackage.
    """
    return arg

# Add your undefined template variables to call 'undefined' method above
JINJA_CONFIG = {'pin_subpackage'        : undefined,
                'compiler'              : undefined,
                'base_mpich'            : undefined('mpich'),
                'base_mpicc'            : undefined('mpicc'),
                'base_mpicxx'           : undefined('mpicxx'),
                'base_mpifort'          : undefined('mpifort'),
                'moose_libgfortran'     : undefined('libgfortran'),
                'moose_libgfortran5'    : undefined('libgfortran5'),
                'moose_hdf5'            : undefined('hdf5'),
                'moose_ld64'            : undefined('ld64'),
                'moose_libxt'           : undefined('ld64'),
                'moose_libsm'           : undefined('ld64'),
                'moose_libx11'          : undefined('libx11'),
                'moose_libice'          : undefined('libice'),
                'moose_libxext'         : undefined('libxext'),
                'moose_mesa_libgl'      : undefined('mesa_libgl'),
                'moose_xorg_x11'        : undefined('xorg_x11'),
                'moose_libglu'          : undefined('libglu'),
                'moose_mesalib'         : undefined('mesalib'),
                'moose_mpich'           : undefined('moose-mpich'),
                'moose_petsc'           : undefined('moose-petsc'),
                'moose_libmesh_vtk'     : undefined('moose-libmesh-vtk'),
                'moose_libmesh'         : undefined('moose-libmesh'),
                }
### End Beautify global

class Versioner:
    """ generates reproducible versions (hashes) for moose apps and moose dependencies """
    def __init__(self):
        self.entities = TRACKING_LIBRARIES
        self.yaml_file = None

    def output_cli(self, args):
        """ performs command line actions """
        args = self.parse_args(args, self.entities)
        self.check_args(args)

        meta = self.version_meta(args.commit)[args.library]
        if args.json:
            return json.dumps(meta)
        if args.yaml:
            return yaml.dump(meta, default_flow_style=False)

        return meta['hash']

    def get_yamlcontents(self, commit):
        """ load yaml file contents at time of suppllied commit """
        # Load the yaml file at the given commit; the location changed
        # from module_hash.yaml -> versioner.yaml at changed_commit
        changed_commit = '2bd844dc5d4de47238eab94a3a718e9714592de1'
        if self.git_ancestor(changed_commit, commit) and commit != changed_commit:
            _file = 'versioner.yaml'
        else:
            _file = 'module_hash.yaml'
        yaml_file = os.path.abspath(os.path.join(MOOSE_DIR, "scripts", _file))
        try:
            yaml_contents = yaml.safe_load(self.git_file(yaml_file, commit))
            self.entities = yaml_contents['packages'].keys()
            return yaml_contents
        except FileNotFoundError:
            print(f'fatal: {yaml_file} not found')
            sys.exit(1)
        except yaml.scanner.ScannerError:
            print(f'fatal: {yaml_file} parsing error')
            sys.exit(1)

    @staticmethod
    def parse_args(argv, entities):
        """ parses arguments """
        parser = argparse.ArgumentParser(description='Supplies a hash for a given library')
        parser.add_argument('library', nargs='?', metavar='library', choices=entities,
                            help=f'choose from: {", ".join(entities)}', default='moose')
        parser.add_argument('commit', nargs='?', metavar='commit', default='HEAD',
                            help='default %(default)s')
        parser.add_argument('-q', '--quiet', action='store_true', default=False,
                            help='Do not print warnings')
        parser.add_argument('--json', action='store_true', default=False,
                            help='Output in JSON format (itemized information)')
        parser.add_argument('--yaml', action='store_true', default=False,
                            help='Output in YAML format (itemized information)')
        return parser.parse_args(argv)

    @staticmethod
    def check_args(args):
        """ checks command line options """
        if args.json and args.yaml:
            print('Cannot use --json and --yaml together')
            sys.exit(1)

        if not Versioner.is_git_object(args.commit):
            print(f'{args.commit} is not a commit in {MOOSE_DIR}')
            sys.exit(1)

    @staticmethod
    def is_git_object(commit_like, repo_dir=MOOSE_DIR):
        """ checks whether or not the object is a valid git object (hash, etc) """
        command = ['git', 'show', commit_like]
        try:
            subprocess.run(command, stdout=subprocess.DEVNULL,
                           stderr=subprocess.PIPE, check=True, cwd=repo_dir)
        except subprocess.CalledProcessError as cpe:
            if cpe.returncode == 128 and 'not in the working tree' in cpe.stderr.decode('utf-8'):
                return False
        except Exception as ex:
            raise ex
        return True

    @staticmethod
    def git_hash(file, commit, repo_dir=MOOSE_DIR):
        """ gets the git hash for the given file at the given commit """
        file = file.replace(repo_dir, '.')
        command = ['git', 'ls-tree', commit, file]
        out = subprocess.check_output(command, cwd=repo_dir).decode('utf-8').split()
        if len(out) == 4:
            return out[2]
        # pylint: disable=broad-exception-raised
        raise Exception(f'Failed to obtain git hash for {file} in {repo_dir} at {commit}')

    @staticmethod
    def git_ancestor(maybe_ancestor, descendant, repo_dir=MOOSE_DIR):
        """ checks whether or not the given commit is an ancestor """
        command = ['git', 'merge-base', '--is-ancestor', maybe_ancestor, descendant]
        try:
            subprocess.run(command, check=True, cwd=repo_dir)
        except subprocess.CalledProcessError as cpe:
            if cpe.returncode == 1: # code 1 means no
                return False
            raise cpe
        except Exception as ex:
            raise ex
        return True

    @staticmethod
    def git_file(file, commit, repo_dir=MOOSE_DIR, allow_missing=False):
        """ gets the contents of a file at a given git commit """
        file = file.replace(repo_dir, '.')
        command = ['git', 'show', f'{commit}:{file}']
        process = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                 cwd=repo_dir, check=False)
        if process.returncode != 0:
            error = process.stderr.decode('utf-8')

            # File missing error
            missing_errors = ['does not exist', 'exists on disk']
            if allow_missing:
                for allowed in missing_errors:
                    if allowed in error:
                        return None
            # pylint: disable=broad-exception-raised
            raise Exception(f'Failed to load {file} in {repo_dir} at {commit}')

        return process.stdout.decode('utf-8')

    @staticmethod
    def parse_jinja(jinja_template):
        """ read jinja_template and return proper yaml (harmless if not jinja) """
        # pylint: disable=unused-argument
        env = jinja2.Environment(loader = jinja2.DictLoader({'' : jinja_template }),
                                 trim_blocks=True,
                                 lstrip_blocks=True)
        meta_template = env.get_template('')
        meta_render = meta_template.render(JINJA_CONFIG)
        return meta_render

    def do_sortlist(self, commit):
        """ determine if we need to sort the infuential file list """
        changed_commit = '0e0785ee8a25742715b49bc26871117b788e7190'
        if self.git_ancestor(changed_commit, commit) and commit != changed_commit:
            return True
        return False

    def influential_dict(self, packages_yaml, parent=None, library=None, recursive_meta=None):
        """ build and return influential dictionary """
        # key descriptors to be treated as control identifiers
        key_descriptors = ['dependencies', 'influential']
        # key name for infuential file list value
        dep_key = 'influential'
        if recursive_meta is None:
            recursive_meta = {}
        for descriptor, values in packages_yaml.items():
            # 'values' is a dictionary with more items to discover (recurse into)
            if isinstance(values, dict):
                # 'descriptor' is actually a library we wish to track
                if descriptor not in key_descriptors:
                    recursive_meta[descriptor] = {}
                # recursive inspection of descriptors_yaml[library], preserve history (grow)
                self.influential_dict(packages_yaml[descriptor],
                                      parent=library,
                                      library=descriptor,
                                      recursive_meta=recursive_meta)
            # no more dictionaries to recurse into
            elif descriptor in key_descriptors:
                # recursive descriptor dictionary (apptainer meta)
                if parent is not None:
                    _lib_dict = recursive_meta[parent]
                else:
                    _lib_dict = recursive_meta[library]
                # we are inside a library with dependency(s)
                if descriptor == 'dependencies':
                    for dep in packages_yaml[descriptor]:
                        if dep in recursive_meta.keys():
                            _lib_dict[dep_key] = recursive_meta[library].get(dep_key, [])
                            _lib_dict[descriptor] = recursive_meta[library].get(descriptor, [])
                            _lib_dict[dep_key].extend(recursive_meta[dep][dep_key])
                            _lib_dict[descriptor].append(dep)
                # anything else (influential files at time of writing)
                else:
                    _lib_dict[descriptor] = recursive_meta[library].get(descriptor, [])
                    _lib_dict[descriptor].extend(packages_yaml[descriptor])
        return recursive_meta

    @staticmethod
    def augment_dictionaries(base_meta, library):
        """ tack on emtpy child key=values to suppied dictionary key """
        child = base_meta[library]
        child['hash_list'] = []
        child['hash_table'] = {}
        child['hash'] = None
        child['conda'] = {}
        return child

    def version_meta(self, commit='HEAD'):
        """ populate and return dictionary making up the contents involved
        with generating hashes """
        # pylint: disable=too-many-locals
        if not self.is_git_object(commit):
            # pylint: disable=broad-exception-raised
            raise Exception(f'{commit} is not a commit in {MOOSE_DIR}')

        # load versioner.yaml contents
        packages = self.get_yamlcontents(commit)['packages']
        sort_list = self.do_sortlist(commit)

        # Use dependencies listed in yaml file
        if sort_list:
            influential_meta = self.influential_dict(packages)
        # Use OrderedDict method
        else:
            influential_meta = OrderedDict()
            file_list = []
            for package, influential in packages.items():
                if isinstance(influential, dict):
                    file_list.extend(influential['influential'])
                else:
                    file_list.extend(influential)
                influential_meta[package] = {'influential' : file_list.copy()}

        # Cache the git hashes for influential files; we could
        # check the same file multiple times, which is surprisingly
        # not cheap
        git_hash_cache = {}

        for package in self.entities:
            is_app = package == 'app'
            if is_app:
                app_name, _, app_hash = self.get_app()
                if app_name is None:
                    continue
            if package not in packages:
                continue

            package_meta = self.augment_dictionaries(influential_meta, package)
            influential_files = package_meta['influential']

            if sort_list:
                # remove duplicates and then sort
                influential_files = list(set(influential_files))
                influential_files.sort()

            for influential_file in influential_files:
                if influential_file not in git_hash_cache:
                    git_hash_cache[influential_file] = self.git_hash(influential_file, commit)

                file_hash = git_hash_cache[influential_file]
                package_meta['hash_table'][influential_file] = file_hash
                package_meta['hash_list'].append(file_hash)
                # If this is the package/meta.yaml file, render the jinja template
                if influential_file.find(f'{package}{os.path.sep}meta.yaml') != -1:
                    package_meta['conda'] = self.conda_meta(package, influential_file, commit)

            package_hash = app_hash if is_app else self.get_hash(package_meta['hash_list'])
            package_meta['hash'] = package_hash

            if 'apptainer' in packages[package]:
                package_meta['apptainer'] = self.apptainer_meta(app_name if is_app else package,
                                                                packages[package]['apptainer'],
                                                                package_hash,
                                                                is_app)
        return influential_meta

    @staticmethod
    def get_app():
        """ gets the current application name/dir/commit the cwd is in, if any """
        # If we're not within a git rep, we're not within an app
        tree_command = ['git', 'rev-parse', '--is-inside-work-tree']
        process = subprocess.run(tree_command, stdout=subprocess.PIPE,
                                 stderr=subprocess.PIPE, check=False)
        if process.returncode != 0:
            return None, None, None

        root_command = ['git', 'rev-parse', '--show-toplevel']
        git_root = subprocess.check_output(root_command, encoding='utf-8').rstrip()
        app_name = os.path.basename(git_root).rstrip().lower()

        # If we're nested within MOOSE_DIR, we're moose_combined app
        if app_name == 'moose':
            app_name = 'moose-combined'

        hash_command = ['git', 'rev-parse', 'HEAD']
        git_hash = subprocess.check_output(hash_command, encoding='utf-8').rstrip()[0:7]

        return app_name, git_root, git_hash

    @staticmethod
    def conda_meta_jinja(contents):
        """
        Gets the name, version, and build from jinja-parsed conda meta.yaml file
        """
        meta = yaml.safe_load(Versioner.parse_jinja(contents))
        name = meta['package']['name']
        version = meta['package']['version']
        build = meta['build'].get('string', None)
        return name, version, build, meta

    @staticmethod
    def conda_meta(package, influential, commit):
        """ produces the conda meta entry """
        # Read the conda-build jinja2 styled template
        contents = Versioner.git_file(influential, commit)
        name, version, build, meta = Versioner.conda_meta_jinja(contents)

        # Make sure the string is build_<NUMBER>
        build_re = re.search(r'^build\_([0-9]+)$', build)
        if not build_re:
            print(f'fatal: {package} conda build string not understood')
            sys.exit(1)

        return {'name': name,
                'version': version,
                'build': int(build_re.group(1)),
                'install': f'{name}={version}={build}',
                'meta': meta}

    @staticmethod
    def apptainer_meta(package, package_apptainer_entry, package_hash, is_app):
        """ produces the apptainer meta entry """
        # Clean up the app name if it's an app
        package = package.rstrip().lower() if is_app else package
        # The def file isn't named after the app, it's always "app"
        def_package = 'app' if is_app else package

        name_base = package
        if package in ['mpich', 'petsc', 'libmesh', 'wasp']:
            name_base = f'moose-{name_base}'
        name_suffix = platform.machine()
        name = f'{name_base}-{name_suffix}'

        meta = {}

        if package_apptainer_entry is not None:
            meta.update(package_apptainer_entry)

        meta.update({'name': name,
                     'name_base': name_base,
                     'name_suffix': name_suffix,
                     'tag': package_hash,
                     'uri': f'{name}:{package_hash}',
                     'def': os.path.realpath(os.path.join(MOOSE_DIR, f'apptainer/{def_package}.def'))})
        return meta

    @staticmethod
    def get_hash(hash_list):
        """ return a seven character hash for given list of strings """
        return hashlib.md5(''.join(hash_list).encode('utf-8')).hexdigest()[:7]

if __name__ == '__main__':
    print(Versioner().output_cli(sys.argv[1:]))
