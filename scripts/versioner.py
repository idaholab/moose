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
import yaml
import json
import jinja2
import copy
from graphlib import TopologicalSorter
from dataclasses import dataclass
from typing import Optional, Tuple, Union
import concurrent.futures

@dataclass
class ApptainerPackage:
    """
    Data class that stores the information about a package
    that is an apptainer package
    """
    # Name of apptainer container
    name: str
    # Name base (no variants) of the apptainer container
    name_base: str
    # Name suffix (arch) of the apptainer container
    name_suffix: str
    # The apptainer package this package depends on (if any)
    from_name: Union[str, None]
    # Tag (version for apptainer)
    tag: str
    # URI for the container (name:tag)
    uri: str
    # Path to the definition file for this container
    def_path: str

    def to_dict(self) -> dict:
        return self.__dict__

@dataclass
class CondaPackage:
    """
    Data class that stores the conda information about a package
    """
    # Name of the conda package
    name: str
    # version identifier for meta.yaml
    version: str
    # build/number identifier for meta.yaml
    build_number: Union[int, None]
    # build/string identifier for meta.yaml
    build_string: str
    # The conda install string for this package (conda install ...)
    install: str
    # Path to the meta yaml file
    meta_yaml: str
    # Path to the build config file
    build_yaml: str
    # The contents of the meta yaml
    meta: dict

    def to_dict(self) -> dict:
        result = copy.deepcopy(self.__dict__)
        del result['meta'] # hide this; it's loud
        return result

@dataclass
class Package:
    """
    Data class that stores all of the information about a package
    """
    # Name of the package
    name: str
    # The global version for the package
    version: str
    # The build number for the package (if any)
    build_number: Union[int, None]
    # The full version (version + build_number)
    full_version: str
    # The hash for the packaged based on influential files
    hash: str
    # The influential files
    influential: list[str]
    # The Packages this package depends on
    dependencies: list
    # The apptainer information for this package (if any)
    apptainer: Union[ApptainerPackage, None]
    # The conda information for this package
    conda: Union[CondaPackage, None]
    # Whether or not the package is the "app" package
    is_app: bool

    def to_dict(self) -> dict:
        result = {}
        for key, value in self.__dict__.items():
            if key in ['apptainer', 'conda'] and value is not None:
                result[key] = value.to_dict()
            elif key == 'dependencies':
                result[key] = [package.name for package in self.dependencies]
            else:
                result[key] = value
        return result

@dataclass
class AppInfo:
    """
    Data class that stores the information about an application
    """
    # Name of the application
    name: str
    # Git root path for the application
    git_root: str
    # Current hash for the application repo
    hash: str

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

    # def verify_recipes(self, args) -> str:
    #     """ provide hints as to version and build information for all conda stack libraries """
    #     red    = '\033[91m'
    #     yellow = '\033[93m'
    #     green  = '\033[92m'
    #     bright = '\033[1m'
    #     reset  = '\033[0m'
    #     empty  = {'version' : 'not yet tracked', 'build': 'n/a'}
    #     head = self.get_packages('HEAD')
    #     base = self.get_packages(args.verify[0])
    #     formatted_output = ''
    #     warn_moose = ''
    #     name_fill = 10
    #     version_fill = 13
    #     for package in head:
    #         name_fill = max(name_fill, len(head[package]['conda'].get('name', '')))
    #         version_fill = max(version_fill, len(base[package]['conda'].get('version', '')))

    #     for name, package in head.items():
    #         # we do not care about app or non-conda packages
    #         if package.is_app or not package.conda:
    #             continue

    #         # create shorter dictionaries with only the things we need
    #         hc = package.conda
    #         base_package = base[name]
    #         bc = base_package.conda

    #         # new libraries being added or tracked in this PR
    #         if not _bc:
    #             _bc = dict(empty)
    #         _bc['hash'] = base[package]['hash']

    #         # string formatting shenanigans
    #         n_fill = f'{"".rjust((name_fill - len(_hc["name"])) + 2," ")}'
    #         v_fill = f'{"".rjust((version_fill - len(_hc["version"])) + 2," ")}'
    #         if _bc['build'] == 'n/a':
    #             v_fill = f'{"".rjust((version_fill - len(_bc["version"]))," ")}'

    #         # HASH MATCH, nothing to do
    #         if _hc['hash'] == _bc['hash']:
    #             print(f'{_hc["name"]}:{n_fill}no changes')
    #             continue

    #         # version/build has not changed, yet some influential file suggests it should
    #         conda_base=f'conda{os.path.sep}{package}{os.path.sep}'
    #         if (_hc['version'], _hc['build']) == (_bc['version'], _bc['build']):
    #             formatted_output+=(f'{_hc["name"]}:{n_fill}{red}'
    #                                f'{_hc["version"]}{reset} build: '
    #                                f'{red}{_hc["build"]}{reset}\n')

    #         # a combo issue of having versioner begin tracking an existing library while
    #         # also mucking with its influentials files, but forgetting to update the
    #         # version/build.
    #         elif not (self.git_is_diff(commit=args.verify[0],
    #                                file=f'{conda_base}meta.yaml') or
    #               self.git_is_diff(commit=args.verify[0],
    #                                file=f'{conda_base}conda_build_config.yaml')):
    #             formatted_output+=(f'{_hc["name"]}:{n_fill}{red}'
    #                                f'{_hc["version"]}{reset} build: '
    #                                f'{red}{_hc["build"]}{reset} '
    #                                f'{v_fill}{v_fill} {red}update required{reset}\n')

    #         # things seem correct, highlight the changed bits in green to aid the user.
    #         else:
    #             print(f'{_hc["name"]}:{n_fill}'
    #                   f'{bright}{_bc["version"]}{reset} build: '
    #                   f'{bright}{_bc["build"]}{reset}{v_fill}to '
    #                   f'{green if _hc["version"] != _bc["version"] else bright}'
    #                   f'{_hc["version"]}{reset} build: '
    #                   f'{green if _hc["build"] != _bc["build"] else bright}{_hc["build"]}{reset}')

    #         # Anything depending on `moose-dev` should go here. Like `moose`.
    #         if package == 'moose-dev':
    #             warn_moose = (f'moose: ({yellow}templated{reset}. Please verify '
    #                           'conda/moose/conda_build_conda.yaml has been updated)')

    #     if formatted_output:
    #         formatted_output+=f'{warn_moose}\n{red}FAIL{reset}'
    #         print(formatted_output)
    #         sys.exit(1)
    #     elif warn_moose:
    #         print(warn_moose)
    #     return f'{green}\nOK{reset}'

    @staticmethod
    def conda_build(library: str) -> None:
        """
        Modifies the conda build yaml files for the given conda package,
        filling in the versioner template strings
        """
        package = Versioner.get_packages('HEAD').get(library)
        if not package:
            Versioner.error(f'Package {library} not found')
        assert package.conda

        for file in [package.conda.meta_yaml, package.conda.build_yaml]:
            contents, replaced = Versioner.augment_conda_yaml(package, file, 'HEAD')
            if not replaced:
                continue
            path = os.path.join(MOOSE_DIR, file)
            with open(path, 'w') as f:
                f.write(contents)
            subprocess.call(['git', 'diff', file], cwd=MOOSE_DIR)

        return None

    def output_summary(self) -> str:
        """ generate summary report that can be used to generate versioner_hash blocks """
        commit = self.git_rev_parse('HEAD')
        output = {commit: {}}
        for package in self.get_packages('HEAD').values():
            if not package.is_app:
                output[commit][package.name] = {'hash': package.hash,
                                                'full_version': package.full_version}
        return yaml.dump(output)

    def output_cli(self, args) -> str:
        """ performs command line actions """
        args = self.parse_args(args, self.entities)
        self.check_args(args)
        if args.summary or args.verify or args.conda_build:
            if self.git_is_diff():
                print('\033[91mWarning\033[0m: you have changes not yet committed. Information'
                      ' displayed may be inaccurate\n')
            if args.summary:
                return self.output_summary()
            if args.verify:
                return self.verify_recipes(args)
            if args.conda_build:
                return self.conda_build(args.conda_build)

        package = self.get_packages(args.commit).get(args.library)
        if not package:
            print(f'{args.library} not tracked in {args.commit}')
            sys.exit(2)
        if args.json:
            return json.dumps(package.to_dict())
        if args.yaml:
            return yaml.dump(package.to_dict())

        return package.full_version

    @staticmethod
    def error(message: str):
        """
        Helper for printing an error message and exits
        """
        print(f'ERROR: {message}')
        sys.exit(1)

    @staticmethod
    def get_packages(ref: str) -> dict[str, Package]:
        """
        Gets the versioned packages for the given git
        reference (typically a commit)
        """
        # Convert reference to a commit; this lets us use things like
        # branches and HEAD
        commit = Versioner.git_rev_parse(ref)

        # Find the path to the versioner config, which changed from
        # module_hash.yaml -> versioner.yaml at changed_commit
        changed_commit = '2bd844dc5d4de47238eab94a3a718e9714592de1'
        yaml_path = 'versioner.yaml'
        if not Versioner.after_commit(changed_commit, commit):
            yaml_path = 'module_hash.yaml'
        yaml_path = os.path.abspath(os.path.join(MOOSE_DIR, 'scripts', yaml_path))

        # Load the versioner yaml file
        try:
            yaml_contents = yaml.safe_load(Versioner.git_file(yaml_path, commit))
        except FileNotFoundError:
            Versioner.error(f'{yaml_path} not found')
        except yaml.scanner.ScannerError:
            Versioner.error(f'{yaml_path} parsing error')
        yaml_packages = yaml_contents['packages']

        # Whether or not we're past the commit where we added
        # dependencies and started sorting the influential files
        changed_commit = '0e0785ee8a25742715b49bc26871117b788e7190'
        old_influential = not Versioner.after_commit(changed_commit, commit)

        # Whether or not we're at the commit where we managed
        # all versions directly in versioner
        changed_commit = 'b05d63c35c7ca2c192a7177954d5cccf1c6dbe09'
        old_versions = not Versioner.after_commit(changed_commit, commit)

        # Load the base info for each package
        all_influential = []
        packages = {}
        for name, values in yaml_packages.items():
            packages[name] = {'name': name, 'apptainer': None, 'conda': None}
            package = packages[name]

            if old_versions:
                package['build_number'] = None
            else:
                package['version'] = values.get('version')
                package['build_number'] = values.get('build_number')

            # Old method for influential files: order based;
            # accumulate influential packages as we progress
            # through the versioner config
            if old_influential:
                package['dependencies'] = []
                if isinstance(values, dict):
                    influential = values['influential']
                else:
                    influential = values
                all_influential.extend(influential)
                package['influential'] = all_influential.copy()
            # New method for influential files: dependency based
            else:
                package['dependencies'] = values.get('dependencies', [])
                package['influential'] = values.get('influential', [])

        # Sort the packages in dependency order (new feature)
        graph = {name: values['dependencies'] for name, values in packages.items()}
        order = tuple(TopologicalSorter(graph).static_order())

        # Add the influential files in dependency order
        for name in order:
            values = packages[name]
            for dependency in values['dependencies']:
                if dependency not in packages:
                    Versioner.error(f'Missing dependency {dependency} for {name}')
                for extend_name in ['dependencies', 'influential']:
                    values[extend_name].extend(packages[dependency][extend_name])

        # Sort the influential files so that the hash is
        # independent of the order that they are defined
        if not old_influential:
            for values in packages.values():
                for key in ['influential', 'dependencies']:
                    entry = list(set(values[key]))
                    entry.sort()
                    values[key] = entry

        # Get the hashes for all influential files; we do this all
        # together so that we can utilize threading
        all_influential = set()
        for values in packages.values():
            all_influential.update(values['influential'])
        all_influential = list(all_influential)
        with concurrent.futures.ThreadPoolExecutor() as executor:
            get_hash = lambda file: Versioner.git_hash(file, commit)
            hashes = list(executor.map(get_hash, all_influential))
        influential_hashes = {}
        for i in range(len(all_influential)):
            influential_hashes[all_influential[i]] = hashes[i]

        # Build the hashes from the influential files. Needs to
        # be done later after dependencies are provessed
        for name, values in packages.items():
            is_app = name == 'app'
            values['is_app'] = is_app
            if is_app: # app uses the direct hash
                app_info = Versioner.get_app_info()
                values['name'] = app_info.name
                values['hash'] = app_info.hash
            else: # not an app
                hashes = [influential_hashes[file] for file in values['influential']]
                hash = hashlib.md5(''.join(hashes).encode('utf-8')).hexdigest()[:7]
            values['hash'] = hash

            if old_versions:
                values['version'] = hash
                values['full_version'] = hash
            else:
                values['full_version'] = f'{values["version"]}'
                if values['build_number'] is not None:
                    values['full_version'] += f'_{values["build_number"]}'

        # Build the Package objects
        package_objects: dict[str, Package] = {}
        for name in order:
            values = packages[name]
            values['dependencies'] = [package_objects[name] for name in values['dependencies']]
            package = Package(**values)
            package_objects[name] = package

        # Add conda and apptainer information now that the
        # Packages are constructed
        for name, package in package_objects.items():
            package.conda = Versioner.get_conda_package(package, commit)

            package_entry = yaml_packages[name]
            if isinstance(package_entry, dict):
                apptainer_entry = yaml_packages[name].get('apptainer')
                if apptainer_entry:
                    package.apptainer = Versioner.get_apptainer_package(package,
                                                                        apptainer_entry)

        return package_objects

    @staticmethod
    def get_apptainer_package(package: Package,
                              apptainer_entry: dict) -> ApptainerPackage:
        """
        Builds the ApptainerPackage information for a given Package.

        The 'apptainer' entry is needed as apptainer_entry
        in order to get information from the versioner.yaml config
        """
        package_name = package.name.lower()

        # The def file isn't named after the app, it's always "app"
        def_package = 'app' if package.is_app else package_name

        name_base = package_name
        if not name_base.startswith('moose-') and not package.is_app:
            name_base = f'moose-{name_base}'
        name_suffix = platform.machine()
        name = f'{name_base}-{name_suffix}'

        values = {'name': name,
                  'from_name': apptainer_entry.get('from'),
                  'name_base': name_base,
                  'name_suffix': name_suffix,
                  'tag': package.full_version,
                  'uri': f'{name}:{package.full_version}',
                  'def_path': os.path.abspath(os.path.join(MOOSE_DIR,
                                                           f'apptainer/{def_package}.def'))}

        return ApptainerPackage(**values)

    @staticmethod
    def get_conda_package(package: Package, commit: str) -> CondaPackage:
        """
        Builds the CondaPackage information for the given Package
        at the given commit.
        """
        # Get the meta.yaml file from the influential files
        # with the same name of the package, if any
        meta_yaml = None
        build_yaml = None
        for file in package.influential:
            if file.endswith(f'{package.name}/meta.yaml'):
                meta_yaml = file
            if file.endswith(f'{package.name}/conda_build_config.yaml'):
                build_yaml = file

        if meta_yaml is None:
            return None

        contents, _ = Versioner.augment_conda_yaml(package, meta_yaml, commit)
        meta = yaml.safe_load(Versioner.render_jinja(contents))

        # Parse from the meta.yaml file
        values = {'name': meta['package']['name'],
                  'version': str(meta['package']['version']),
                  'build_string': meta['build'].get('string', None),
                  'meta_yaml': meta_yaml,
                  'build_yaml': build_yaml,
                  'meta': meta}
        values['install'] = values['version']
        if values['build_string']:
            values['install'] += '=' + values['build_string']
        build_number = meta['build']['number']
        if build_number is not None:
            build_number = int(build_number)
        values['build_number'] = build_number

        return CondaPackage(**values)

    @staticmethod
    def augment_conda_yaml(package: Package,
                           file: os.PathLike,
                           commit: str) -> Tuple[str, bool]:
        """
        Augments a conda yaml file, replacing all
        __VERSIONER_*__ entries
        """
        assert file in package.influential
        contents = Versioner.git_file(file, commit)

        # Loop through all of the dependencies and build out
        # the strings that we would replace
        replacements = []
        prefix = '__VERSIONER_'
        for other_package in [package] + package.dependencies:
            name = other_package.name.replace('-', '_').upper()
            replacements.append((f'{prefix}{name}_VERSION__',
                                 str(other_package.version)))
            replacements.append((f'{prefix}{name}_BUILD_NUMBER__',
                                 str(other_package.build_number)))
        # Do the replacements
        replaced = False
        for from_val, to_val in replacements:
            contents_new = contents.replace(from_val, to_val)
            if contents != contents_new:
                replaced = True
            contents = contents_new

        # We should no longer have any __VERSIONER_ stuff left,
        # otherwise we forgot a dependency
        if prefix in contents:
            Versioner.error(f'Versioner template still exists in {file}. '
                            'You likely forgot to add a dependency')

        return contents, replaced

    @staticmethod
    def parse_args(argv: list[str], entities: list[str]) -> argparse.Namespace:
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
        parser.add_argument('-s','--summary', action='store_true', default=False,
                            help='Output summary as should be entered in versioner_hashes.yaml')
        parser.add_argument('--conda-build', type=str,
                            help='Augment the conda build files for the given package')
        parser.add_argument('-v', '--verify', nargs=1, metavar='base_ref hash', default=None,
                            help='Output version/build number hints against supplied base reference'
                            ' hash')
        return parser.parse_args(argv)

    @staticmethod
    def check_args(args: argparse.Namespace) -> None:
        """ checks command line options """
        actions = ['json', 'yaml', 'summary', 'conda_build', 'verify']
        for action in actions:
            if getattr(args, action):
                for other_action in actions:
                    if action != other_action and getattr(args, other_action):
                        Versioner.error(f"Cannot use --{action} and "
                                        f"--{other_action} together")

        if args.verify and args.verify == 'HEAD':
            Versioner.error('You cannot verify against HEAD. You must choose a hash'
                            ' (preferably something like upstream/master)')

    @staticmethod
    def git_is_diff(repo_dir: os.PathLike = MOOSE_DIR,
                    commit: Optional[str] = None,
                    file: Optional[str] = None) -> bool:
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
    def git_hash(file: os.PathLike,
                 commit: str,
                 repo_dir: os.PathLike = MOOSE_DIR) -> str:
        """ gets the git hash for the given file at the given commit """
        file = file.replace(repo_dir, '.')
        command = ['git', 'ls-tree', commit, file]
        out = subprocess.check_output(command, cwd=repo_dir, text=True).split()
        if len(out) == 4:
            return out[2]
        # pylint: disable=broad-exception-raised
        raise Exception(f'Failed to obtain git hash for {file} in {repo_dir} at {commit}')

    @staticmethod
    def git_ancestor(maybe_ancestor: str,
                     descendant: str,
                     repo_dir: os.PathLike = MOOSE_DIR) -> bool:
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
    def after_commit(base: str, commit: str) -> bool:
        """
        Checks if the given commit is after the base commit
        """
        return Versioner.git_ancestor(base, commit) and base != commit

    @staticmethod
    def git_file(file: str,
                 commit: str,
                 repo_dir: os.PathLike = MOOSE_DIR,
                 allow_missing: bool = False) -> str:
        """ gets the contents of a file at a given git commit """
        if os.path.isabs(file):
            relative = os.path.relpath(file, MOOSE_DIR)
            if relative.startswith('..'):
                raise Exception(f'Supplied path {file} is not in {MOOSE_DIR}')
            file = relative

        command = ['git', 'show', f'{commit}:{file}']
        process = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                 cwd=repo_dir, check=False, text=True)
        if process.returncode != 0:
            error = process.stderr

            # File missing error
            missing_errors = ['does not exist', 'exists on disk']
            if allow_missing:
                for allowed in missing_errors:
                    if allowed in error:
                        return None
            # pylint: disable=broad-exception-raised
            raise Exception(f'Failed to load {file} in {repo_dir} at {commit}')

        return process.stdout

    @staticmethod
    def render_jinja(contents: str) -> str:
        """ read jinja_template and return it rendered (harmless if not jinja) """
        # pylint: disable=unused-argument
        env = jinja2.Environment(loader = jinja2.DictLoader({'' : contents}),
                                 trim_blocks=True,
                                 lstrip_blocks=True)
        meta_template = env.get_template('')
        meta_render = meta_template.render(JINJA_CONFIG)
        return meta_render

    @staticmethod
    def get_app_info() -> Union[AppInfo, None]:
        """ gets the current application name/dir/commit the cwd is in, if any """
        # If we're not within a git rep, we're not within an app
        tree_command = ['git', 'rev-parse', '--is-inside-work-tree']
        process = subprocess.run(tree_command, stdout=subprocess.PIPE,
                                 stderr=subprocess.PIPE, check=False)
        if process.returncode != 0:
            return None

        root_command = ['git', 'rev-parse', '--show-toplevel']
        git_root = subprocess.check_output(root_command, encoding='utf-8').rstrip()
        app_name = os.path.basename(git_root).rstrip().lower()
        git_hash = str(Versioner.git_rev_parse('HEAD', os.getcwd()))

        return AppInfo(name=app_name,
                       git_root=git_root,
                       hash=git_hash[0:7])

    @staticmethod
    def git_rev_parse(ref: str, dir: os.PathLike = MOOSE_DIR) -> str:
        """
        Calls `git rev-parse` with the given reference
        """
        cmd = ['git', 'rev-parse', ref]
        try:
            return subprocess.check_output(cmd, text=True, cwd=dir).rstrip()
        except subprocess.CalledProcessError as e:
            if e.returncode == 128:
                raise Exception(f'Reference {ref} is not valid in repo {dir}')
            raise

if __name__ == '__main__':
    result = Versioner().output_cli(sys.argv[1:])
    if result:
        print(result)
