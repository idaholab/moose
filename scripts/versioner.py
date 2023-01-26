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
import tempfile
import json
from collections import OrderedDict
import yaml
from jinja2 import Environment, DictLoader

MOOSE_DIR = os.environ.get('MOOSE_DIR',
                           os.path.abspath(os.path.join(os.path.dirname(
                               os.path.realpath(__file__)), '..')))

class Versioner:
    """ generates reproducible versions (hashes) for moose apps and moose dependencies """
    def __init__(self):
        self.yaml_file = os.path.abspath(os.path.join(MOOSE_DIR, "scripts", "versioner.yaml"))
        try:
            with open(f'{self.yaml_file}', 'r', encoding='utf-8') as rc_file:
                self.entities = list(yaml.safe_load(rc_file)['packages'].keys())
        except FileNotFoundError:
            print(f'fatal: {self.yaml_file} not found')
            sys.exit(1)
        except yaml.scanner.ScannerError:
            print(f'fatal: {self.yaml_file} parsing error')
            sys.exit(1)

    def output_cli(self, args):
        """ performs command line actions """
        args = self.parse_args(args, self.entities)
        self.check_args(args)

        meta = self.meta(args.commit)[args.library]
        if args.json:
            return json.dumps(meta)
        if args.yaml:
            return yaml.dump(meta, default_flow_style=False)

        return meta['hash']

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

            raise Exception(f'Failed to load {file} in {repo_dir} at {commit}')

        return process.stdout.decode('utf-8')

    @staticmethod
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

    def meta(self, commit='HEAD'):
        """ populate and return dictionary making up the contents involved
        with generating hashes """
        # pylint: disable=too-many-locals
        if not self.is_git_object(commit):
            raise Exception(f'{commit} is not a commit in {MOOSE_DIR}')

        # Load the yaml file at the given commit; the location changed
        # from module_hash.yaml -> versioner.yaml at changed_commit
        changed_commit = 'bd99d2074e06b720bdc8cd3017f4597a36fd36de'
        if self.git_ancestor(changed_commit, commit) and commit != changed_commit:
            yaml_file = self.yaml_file
        else:
            yaml_file = 'scripts/module_hash.yaml'

        yaml_contents = self.git_file(yaml_file, commit)
        packages = yaml.safe_load(yaml_contents)['packages']

        meta = OrderedDict()
        hash_list = []
        influential_list = []

        for package in self.entities:
            is_app = package == 'app'
            if is_app:
                app_name, _, app_hash = self.get_app()
                if app_name is None:
                    continue
            if package not in packages:
                continue

            meta[package] = {}
            entry = meta[package]

            package_influential = packages[package]
            influential_list.extend(package_influential)
            entry['influential'] = list(influential_list)
            for package_entry in package_influential:
                entry_hash = self.git_hash(package_entry, commit)
                hash_list.append(entry_hash)

            package_hash = app_hash if is_app else self.get_hash(hash_list)
            entry['hash'] = package_hash
            entry['conda'] = self.conda_meta(package, package_influential, commit)
            entry['apptainer'] = self.apptainer_meta(app_name if is_app else package,
                                                     package_hash,
                                                     is_app)

        return meta

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
        for entry in influential:
            if 'meta.yaml' not in entry or package == 'moose':
                continue

            # Read the conda-build jinja2 styled template
            contents = Versioner.git_file(entry, commit)
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
        return None

    @staticmethod
    def apptainer_meta(package, package_hash, is_app):
        """ produces the apptainer meta entry """
        # Clean up the app name if it's an app
        package = package.rstrip().lower() if is_app else package
        # The def file isn't named after the app, it's always "app"
        def_package = 'app' if is_app else package

        name_base = package
        if package in ['mpich', 'petsc', 'libmesh']:
            name_base = f'moose-{name_base}'
        name_suffix = platform.machine()
        name = f'{name_base}-{name_suffix}'
        return {'name': name,
                'name_base': name_base,
                'name_suffix': name_suffix,
                'tag': package_hash,
                'uri': f'{name}:{package_hash}',
                'def': os.path.realpath(os.path.join(MOOSE_DIR, f'apptainer/{def_package}.def'))}

    @staticmethod
    def get_hash(hash_list):
        """ return a seven character hash for given list of strings """
        return hashlib.md5(''.join(hash_list).encode('utf-8')).hexdigest()[:7]

if __name__ == '__main__':
    print(Versioner().output_cli(sys.argv[1:]))
