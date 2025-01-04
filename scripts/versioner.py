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
from collections import OrderedDict, defaultdict
import yaml
import jinja2

MOOSE_DIR = os.environ.get('MOOSE_DIR',
                           os.path.abspath(os.path.join(os.path.dirname(
                               os.path.realpath(__file__)), '..')))

### Unittest Tracking Libraries (versioner_hashes.yaml)
# note: Order is important
TRACKING_LIBRARIES = ['tools', 'mpi', 'petsc', 'libmesh', 'wasp', 'moose-dev', 'app']

### Additional libraries for tracking_libraries, necessary to build the moose-dev Conda stack
# This allows returning a proper verification
# note: If adding to the following list below, see line 64 in
# python/MooseDocs/test/extensions/test_versioner.py
MOOSE_DEV_STACK = set(['libmesh-vtk',
                       'peacock']).union(set(TRACKING_LIBRARIES))

### All Libraries (versioner.yaml)
# This allows returning a hash for any library
# note: If adding to the following list below, see line 64 in
# python/MooseDocs/test/extensions/test_versioner.py
LIBRARIES = set(['seacas',
                 'pyhit',
                 'pprof']).union(set(TRACKING_LIBRARIES).union(MOOSE_DEV_STACK))

### Beautify the output of jinja2 rendered content that may only exists in conda-build scenarios
# pylint: disable=unused-argument
def undefined(arg, *args, **kwargs):
    """
    Handle any number of passed arguments.
    The *, ** args is needed by pin_subpackage.
    """
    return arg

# Read MOOSE_DIR/framework/doc/packages_config.yml to grab defaults
packages_config = os.path.join(MOOSE_DIR, 'framework', 'doc', 'packages_config.yml')
packages_yaml = {'default_mpi' : 'mpich'}
if os.path.exists(packages_config):
    with open(packages_config, 'r') as pkg_file:
        packages_yaml.update(yaml.safe_load(pkg_file))

# Allow jinja template variables to return a value when otherwise it would not
JINJA_CONFIG = {'pin_compatible'        : undefined,
                'pin_subpackage'        : undefined,
                'compiler'              : undefined,
                'mpi'                   : undefined(packages_yaml['default_mpi']),
                'moose_libgfortran'     : undefined('libgfortran'),
                'moose_libgfortran5'    : undefined('libgfortran5'),
                'moose_petsc'           : undefined('moose-petsc'),
                'moose_libmesh_vtk'     : undefined('moose-libmesh-vtk'),
                'moose_libmesh'         : undefined('moose-libmesh'),
                }

class Versioner:
    """ generates reproducible versions (hashes) for moose apps and moose dependencies """
    def __init__(self):
        self.entities = LIBRARIES
        self.yaml_file = None

    def verify_recipes(self, args) ->str:
        """ provide hints as to version and build information for all conda stack libraries """
        red    = '\033[91m'
        yellow = '\033[93m'
        green  = '\033[92m'
        bright = '\033[1m'
        reset  = '\033[0m'
        empty  = {'version' : 'not yet tracked', 'build': 'n/a'}
        head = self.version_meta('HEAD')
        base = self.version_meta(args.verify[0])
        formatted_output = ''
        warn_moose = ''
        name_fill = 10
        version_fill = 13
        for package in head:
            name_fill = max(name_fill, len(head[package]['conda'].get('name', '')))
            version_fill = max(version_fill, len(base[package]['conda'].get('version', '')))

        for package in head:
            # we do not care about app
            if package == 'app':
                continue

            # libraries without a meta.yaml (pyhit for example, which is now wasp)
            if not head[package].get('conda', False):
                continue

            # create shorter dictionaries with only the things we need
            _hc = head[package]['conda']
            _hc['hash'] = head[package]['hash']
            _bc = base[package]['conda']

            # new libraries being added or tracked in this PR
            if not _bc:
                _bc = dict(empty)
            _bc['hash'] = base[package]['hash']

            # string formatting shenanigans
            n_fill = f'{"".rjust((name_fill - len(_hc["name"])) + 2," ")}'
            v_fill = f'{"".rjust((version_fill - len(_hc["version"])) + 2," ")}'
            if _bc['build'] == 'n/a':
                v_fill = f'{"".rjust((version_fill - len(_bc["version"]))," ")}'

            # HASH MATCH, nothing to do
            if _hc['hash'] == _bc['hash']:
                print(f'{_hc["name"]}:{n_fill}no changes')
                continue

            # version/build has not changed, yet some influential file suggests it should
            conda_base=f'conda{os.path.sep}{package}{os.path.sep}'
            if (_hc['version'], _hc['build']) == (_bc['version'], _bc['build']):
                formatted_output+=(f'{_hc["name"]}:{n_fill}{red}'
                                   f'{_hc["version"]}{reset} build: '
                                   f'{red}{_hc["build"]}{reset}\n')

            # a combo issue of having versioner begin tracking an existing library while
            # also mucking with its influentials files, but forgetting to update the
            # version/build.
            elif not (self.git_is_diff(commit=args.verify[0],
                                   file=f'{conda_base}meta.yaml') or
                  self.git_is_diff(commit=args.verify[0],
                                   file=f'{conda_base}conda_build_config.yaml')):
                formatted_output+=(f'{_hc["name"]}:{n_fill}{red}'
                                   f'{_hc["version"]}{reset} build: '
                                   f'{red}{_hc["build"]}{reset} '
                                   f'{v_fill}{v_fill} {red}update required{reset}\n')

            # things seem correct, highlight the changed bits in green to aid the user.
            else:
                print(f'{_hc["name"]}:{n_fill}'
                      f'{bright}{_bc["version"]}{reset} build: '
                      f'{bright}{_bc["build"]}{reset}{v_fill}to '
                      f'{green if _hc["version"] != _bc["version"] else bright}'
                      f'{_hc["version"]}{reset} build: '
                      f'{green if _hc["build"] != _bc["build"] else bright}{_hc["build"]}{reset}')

            # Anything depending on `moose-dev` should go here. Like `moose`.
            if package == 'moose-dev':
                warn_moose = (f'moose: ({yellow}templated{reset}. Please verify '
                              'conda/moose/conda_build_conda.yaml has been updated)')

        if formatted_output:
            formatted_output+=f'{warn_moose}\n{red}FAIL{reset}'
            print(formatted_output)
            sys.exit(1)
        elif warn_moose:
            print(warn_moose)
        return f'{green}\nOK{reset}'

    def output_summary(self, args):
        """ generate summary report that can be used to generate versioner_hash blocks """
        head = self.version_meta(args.commit, full_hash=True)["app"]["hash"]
        formatted_output = f'{head}: #PR\n'
        for library in TRACKING_LIBRARIES:
            if library == 'app':
                continue
            meta = self.version_meta(args.commit).get(library, {})
            meta_hash = meta['hash']
            formatted_output+=f'  {library}: {meta_hash}\n'
        return formatted_output

    def output_cli(self, args):
        """ performs command line actions """
        args = self.parse_args(args, self.entities)
        self.check_args(args)
        if args.summary or args.verify:
            if self.git_is_diff():
                print('\033[91mWarning\033[0m: you have changes not yet committed. Information'
                      ' displayed may be inaccurate\n')
            if args.summary:
                return self.output_summary(args)
            if args.verify:
                return self.verify_recipes(args)

        meta = self.version_meta(args.commit).get(args.library, {})
        if not meta:
            print(f'{args.library} not tracked in {args.commit}')
            sys.exit(2)
        if args.json:
            return json.dumps(meta)
        if args.yaml:
            return yaml.dump(meta, default_flow_style=False)

        return meta['hash']

    def get_yamlcontents(self, commit):
        """ load yaml file contents at time of supplied commit """
        # Load the yaml file at the given commit; the location changed
        # from module_hash.yaml -> versioner.yaml at changed_commit
        changed_commit = '2bd844dc5d4de47238eab94a3a718e9714592de1'
        if commit == 'HEAD':
            _file = 'versioner.yaml'
        elif self.git_ancestor(changed_commit, commit) and commit != changed_commit:
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
                            help=f'choose from: {", ".join(entities)}', default='moose-dev')
        parser.add_argument('commit', nargs='?', metavar='commit', default='HEAD',
                            help='default %(default)s')
        parser.add_argument('-q', '--quiet', action='store_true', default=False,
                            help='Do not print warnings')
        parser.add_argument('--json', action='store_true', default=False,
                            help='Output in JSON format (itemized information)')
        parser.add_argument('--yaml', action='store_true', default=False,
                            help='Output in YAML format (itemized information)')
        parser.add_argument('-s','--summary',action='store_true', default=False,
                            help='Output summary as should be entered in versioner_hashes.yaml')
        parser.add_argument('-v', '--verify', nargs=1, metavar='base_ref hash', default=None,
                            help='Output version/build number hints against supplied base reference'
                            ' hash')
        return parser.parse_args(argv)

    @staticmethod
    def check_args(args):
        """ checks command line options """
        if args.json and args.yaml:
            print('Cannot use --json and --yaml together')
            sys.exit(1)

        if args.verify and args.verify == 'HEAD':
            print('You cannot verify against HEAD. You must choose a hash'
                  ' (preferably something like upstream/master)')
            sys.exit(1)

        if not Versioner.is_git_object(args.commit):
            print(f'{args.commit} is not a commit in {MOOSE_DIR}')
            sys.exit(1)

    @staticmethod
    def git_is_diff(repo_dir=MOOSE_DIR, commit=None, file=None) ->bool:
        """ returns bool on diff changes present in MOOSE_DIR """
        command = ['git', 'diff']
        if commit is not None:
            command.append(commit)
        if file is not None:
            command.append(file)
        try:
            diff = subprocess.run(command, stdout=subprocess.PIPE,
                                  stderr=subprocess.DEVNULL, check=True, cwd=repo_dir)
        except Exception as ex:
            raise ex
        if len(diff.stdout):
            return True
        return False

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
        if os.path.isabs(file):
            relative = os.path.relpath(file, MOOSE_DIR)
            if relative.startswith('..'):
                raise Exception(f'Supplied path {file} is not in {MOOSE_DIR}')
            file = relative

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
        if commit == 'HEAD':
            return True
        elif self.git_ancestor(changed_commit, commit) and commit != changed_commit:
            return True
        return False

    @staticmethod
    def influential_dict(packages_yaml, library=None, build_dict=None) ->dict:
        _d = defaultdict(dict) if build_dict == None else build_dict
        if library is not None and not len(_d[library]['influential']):
            _d[library]['influential'].extend(packages_yaml[library].get('influential', []))
            _d[library]['dependencies'].extend(packages_yaml[library].get('dependencies', []))
            for dep_package in packages_yaml[library].get('dependencies', []):
                if dep_package not in _d:
                    _d.update({ dep_package : {'influential' : [], 'dependencies' : []} })
                _tmp = Versioner.influential_dict(packages_yaml, dep_package, _d)
                _d[library]['influential'].extend(_tmp[dep_package]['influential'])
        if library == None:
            for package in packages_yaml:
                _d.update({ package : {'influential' : [], 'dependencies' : [] } })
                _d.update(Versioner.influential_dict(packages_yaml, package, _d))
            return _d
        return _d

    @staticmethod
    def augment_dictionaries(base_meta, library):
        """ tack on empty child key=values to supplied dictionary key """
        child = base_meta[library]
        child['hash_list'] = []
        child['hash_table'] = {}
        child['hash'] = None
        child['conda'] = {}
        return child

    def version_meta(self, commit='HEAD', full_hash=False):
        """
        populate and return dictionary making up the contents involved with generating hashes
        """
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
                app_name, _, app_hash = self.get_app(full_hash=full_hash)
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
                # If this is the */meta.yaml file, render the jinja template
                if influential_file.find(f'{package}{os.path.sep}meta.yaml') != -1:
                    package_meta['conda'] = self.conda_meta(influential_file, commit)

            package_hash = app_hash if is_app else self.get_hash(package_meta['hash_list'])
            package_meta['hash'] = package_hash

            if 'apptainer' in packages[package]:
                package_meta['apptainer'] = self.apptainer_meta(app_name if is_app else package,
                                                                packages[package]['apptainer'],
                                                                package_hash,
                                                                is_app)
        return influential_meta

    @staticmethod
    def get_app(full_hash=False):
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

        hash_command = ['git', 'rev-parse', 'HEAD']
        git_hash = subprocess.check_output(hash_command, encoding='utf-8').rstrip()

        return app_name, git_root, f'{git_hash if full_hash else git_hash[0:7]}'

    @staticmethod
    def conda_meta_jinja(contents):
        """
        Gets the name, version, and build from jinja-parsed conda meta.yaml file
        """
        meta = yaml.safe_load(Versioner.parse_jinja(contents))
        name = meta['package']['name']
        version = meta['package']['version']
        build_string = meta['build'].get('string', None)
        build_number = meta['build']['number']
        return name, version, build_string, build_number, meta

    @staticmethod
    def conda_meta(influential, commit):
        """ produces the conda meta entry """
        # Read the conda-build jinja2 styled template
        contents = Versioner.git_file(influential, commit)
        name, version, build_string, build_number, meta = Versioner.conda_meta_jinja(contents)

        version_and_build = version
        if build_string is not None:
            version_and_build += f'={build_string}'

        return {'name': name,
                'version': version,
                'build': int(build_number) if build_number is not None else None,
                'version_and_build': version_and_build,
                'install': f'{name}={version_and_build}',
                'meta': meta}

    @staticmethod
    def apptainer_meta(package, package_apptainer_entry, package_hash, is_app):
        """ produces the apptainer meta entry """
        # Clean up the app name if it's an app
        package = package.rstrip().lower() if is_app else package
        # The def file isn't named after the app, it's always "app"
        def_package = 'app' if is_app else package

        name_base = package
        if package in ['mpi', 'petsc', 'libmesh', 'wasp']:
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
                     'def': os.path.realpath(os.path.join(MOOSE_DIR,
                                                          f'apptainer/{def_package}.def'))})
        return meta

    @staticmethod
    def get_hash(hash_list):
        """ return a seven character hash for given list of strings """
        return hashlib.md5(''.join(hash_list).encode('utf-8')).hexdigest()[:7]

if __name__ == '__main__':
    print(Versioner().output_cli(sys.argv[1:]))
