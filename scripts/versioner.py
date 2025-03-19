#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
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
import itertools
import re
import concurrent.futures
from graphlib import TopologicalSorter
from dataclasses import dataclass
from typing import Optional, Tuple, Union
from datetime import date

import yaml
import jinja2

SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
MOOSE_DIR = os.environ.get('MOOSE_DIR',
                           os.path.abspath(os.path.join(SCRIPT_DIR, '..')))

try:
    from mooseutils import colorText
except ModuleNotFoundError:
    sys.path.append(os.path.join(MOOSE_DIR, 'python'))
    from mooseutils import colorText

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

    def output(self) -> dict:
        """
        Helper for outputting this object to screen
        """
        return self.__dict__.copy()

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
    # Path to the conda build directory
    conda_dir: str
    # The meta yaml contents
    meta: dict

    def output(self) -> dict:
        """
        Helper for outputting this object to screen
        """
        out = self.__dict__.copy()
        out['meta'] = 'hidden'
        return out

@dataclass
class Package:
    """
    Data class that stores all of the information about a package
    """
    # Name of the package
    name: str
    # The configuration entry from versioner.yaml
    config: dict
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
    # The templates for this package (template -> output)
    templates: dict[str, str]
    # The apptainer information for this package (if any)
    apptainer: Union[ApptainerPackage, None]
    # The conda information for this package
    conda: Union[CondaPackage, None]
    # Whether or not the package is the "app" package
    is_app: bool

    def output(self) -> dict:
        """
        Helper for outputting this object to screen
        """
        out = {}
        for key, value in self.__dict__.copy().items():
            if key in ['apptainer', 'conda'] and value is not None:
                out[key] = value.output()
            elif key == 'dependencies':
                out[key] = [package.name for package in self.dependencies]
            elif key in ['config']:
                continue
            else:
                out[key] = value
        return out

@dataclass
class TemplateConfig:
    """
    Data class that stores the information about a template
    """
    # The prefix for the template variables
    prefix: str
    # The suffix for the template variables
    suffix: str
    # The variables to replace
    variables: list[str]

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
    with open(packages_config, 'r', encoding="utf-8") as pkg_file:
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

    @staticmethod
    def match_date(version) -> Union[Tuple, None]:
        """
        Matches a date to the given string, if any
        """
        search = re.search(r'(\d{4}).(\d{2}).(\d{2})', version)
        if search:
            return (int(search.group(1)), int(search.group(2)), int(search.group(3)))
        return None

    def verify_recipes(self, args) -> None:
        """ provide hints as to version and build information for all conda stack libraries """
        from tabulate import tabulate

        head = self.get_packages('HEAD')
        base = self.get_packages(args.verify[0])
        brief = args.brief

        # Helper for building a result
        def print_result(data, header, keys, summary):
            tablefmt = 'github' if brief else 'rounded_grid'
            contents = ''
            length = 0
            if brief:
                contents = f'{summary}\n'
            if data:
                table = tabulate(data, headers=keys, tablefmt=tablefmt)
                length = len(table.splitlines()[1])
                contents += '\n' + tabulate(data, headers=keys, tablefmt=tablefmt)
            prefix = f'### {header}\n' if brief else (header.center(length))
            print(f'{prefix}\n{contents}')

        # Helper for coloring
        def colorize(value, color):
            return colorText(value, color) if (color and not brief) else value

        # Build template table
        entries = []
        num_templates = 0
        num_templates_failed = 0
        for package in head.values():
            for template_path, to_path in package.templates.items():
                num_templates += 1
                template_contents = Versioner.augment_template(package, template_path)
                contents = Versioner.git_file(to_path, 'HEAD')
                changes = template_contents != contents
                if not changes and brief:
                    continue
                change_text = 'BEHIND' if changes else colorize('OK', 'GREEN')
                entries.append((package.name, change_text, to_path, template_path))
                if changes:
                    num_templates_failed += 1
                    entries[-1] = [colorize(v, 'RED') for v in entries[-1]]

        # Print template table
        print_result(entries,
                     'Versioner templates',
                     ['package', 'status', 'file', 'from'],
                     f'Found {num_templates} templates, {num_templates_failed} failed')
        print()

        # Build version table
        entries = []
        num_packages = 0
        num_packages_changed = 0
        num_packages_failed = 0
        for name, head_package in head.items():
            # we do not care about app or non-conda packages
            if head_package.is_app or not head_package.conda:
                continue

            num_packages += 1
            head_conda = head_package.conda
            base_package = base[name]
            base_conda = base_package.conda

            status = 'OK'
            status_color = 'GREEN'
            package = name
            package_color = None
            hash_color = None
            version_color = None
            build_color = None

            # Hashes are different, something changed
            if base_package.hash != head_package.hash:
                num_packages_changed += 1
                hash_color = 'YELLOW'

                different_version = base_package.version != head_package.version
                different_build = base_package.build_number != head_package.build_number
                if different_version:
                    version_color = 'YELLOW'
                if different_build:
                    build_color = 'YELLOW'

                # Full version is different, which means it was bumped
                if base_conda.install != head_conda.install:
                    status = 'CHANGE'
                    if different_version:
                        base_date = self.match_date(base_package.version)
                        head_date = self.match_date(head_package.version)
                        if head_date is not None:
                            # Check that version date increased
                            if base_date is not None and \
                            date(*head_date) < date(*base_date):
                                version_color = 'RED'
                                status = 'DATE DECREASE'
                            # Version date is not in the future
                            elif date(*head_date) > date.today():
                                version_color = 'RED'
                                status = 'FUTURE DATE'
                        # Version is bumped, but build isn't zero
                        elif head_package.build_number is not None \
                        and head_package.build_number != 0:
                            build_color = 'RED'
                            status = 'BUILD NONZERO'
                    # Version is bumped correctly
                    if status == 'CHANGE':
                        if different_version:
                            version_color = 'GREEN'
                        if different_build:
                            build_color = 'GREEN'
                # Version is not different, forgot to bump
                else:
                    status = 'NEED BUMP'
                    version_color = 'RED'
                    build_color = 'RED'

            # Something went wrong, make status and package red
            if status not in ['OK', 'CHANGE']:
                package_color = 'RED'
                status_color = 'RED'
                num_packages_failed += 1

            if not brief or status != 'OK':
                base_version = base_conda.version
                if base_package.build_number is not None:
                    base_version += f' build {base_package.build_number}'
                head_version = colorize(head_conda.version, version_color)
                if head_package.build_number is not None:
                    head_version += colorize(f' build {head_package.build_number}', build_color)
                entries.append([colorize(name, package_color),
                                colorize(status, status_color),
                                base_package.hash,
                                colorize(head_package.hash, hash_color),
                                base_version,
                                head_version])

        # Print version table
        keys = ['package', 'status', 'hash', 'to hash', 'version', 'to version']
        print_result(entries,
                     'Versioner versions',
                     keys,
                     f'Found {num_packages} packages, {num_packages_changed} changed, '
                     f'{num_packages_failed} failed')

        if num_packages_changed and not brief:
            print(f'\nChanges were found in {num_packages_changed} packages.')
        if num_packages_failed or num_templates_failed:
            print('\nVerification failed.')
            sys.exit(2)
        elif not brief:
            print('\nVerification succeeded.')

    @staticmethod
    def build_templates() -> None:
        """
        Performs the --build-templates action

        Writes all of the templated files
        """
        for package in Versioner.get_packages('HEAD').values():
            for template_path, to_path in package.templates.items():
                abs_to_path = os.path.join(MOOSE_DIR, to_path)
                rel_to_path = os.path.relpath(abs_to_path)
                template_contents = Versioner.augment_template(package, template_path)

                with open(abs_to_path, 'r', encoding="utf-8") as f:
                    to_contents = f.read()

                changed = template_contents != to_contents
                print('MODIFIED ' if changed else 'UNCHANGED', rel_to_path)
                if changed:
                    with open(abs_to_path, 'w', encoding="utf-8") as f:
                        f.write(template_contents)

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
        if args.summary or args.verify or args.build_templates:
            if self.git_is_diff():
                print('\033[91mWARNING\033[0m: You have changes not yet committed. Information'
                      ' displayed may be inaccurate!\n')
            if args.summary:
                return self.output_summary()
            if args.verify:
                return self.verify_recipes(args)
            if args.build_templates:
                return self.build_templates()

        package = self.get_packages(args.commit).get(args.library)
        if not package:
            print(f'{args.library} not tracked in {args.commit}')
            sys.exit(2)
        if args.json:
            return json.dumps(package.output())
        if args.yaml:
            return yaml.dump(package.output())

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

        # Whether or not we're past the commit where we added
        # dependencies and started sorting the influential files
        old_influential = Versioner.using_old_influential(commit)

        # Whether or not we're at the commit where we managed
        # all versions directly in versioner
        old_versions = not Versioner.using_managed_versions(commit)

        # Load the packages from versioner.yaml
        config = Versioner.load_versioner_yaml(commit)['packages']

        all_packages: list[str] = []
        packages: dict[str, Package] = {}
        for name, values in config.items():
            if old_influential:
                # Originally, the values were just a list and not
                # a dict where that list was the influential files;
                # convert those to a dict for consistency
                if isinstance(values, list):
                    values = {'influential': values}
                # Originally, dependencies were implicitly defined
                # based on the ordering in the config
                values['dependencies'] = all_packages.copy()

            package = {'name': Versioner.get_app_info().name if name == 'app' else name,
                       'config': values,
                       'version': values.get('version'),
                       'build_number': values.get('build_number'),
                       'full_version': None,
                       'hash': None,
                       'influential': values.get('influential', []),
                       'dependencies': values.get('dependencies', []),
                       'templates': values.get('templates', {}),
                       'apptainer': None,
                       'conda': values.get('conda'),
                       'is_app': name == 'app'}

            # Append conda folder as influential and the specifically
            # tracked files (which need to be augumented because
            # they have templated variables)
            conda_dir = package['conda']
            if conda_dir:
                package['influential'].extend(Versioner.conda_influential(conda_dir))

            # Make sure dependencies exist
            for dep in package['dependencies']:
                if dep == name:
                    Versioner.error(f'{name} depends on itself')
                if dep not in config:
                    Versioner.error(f'{name} missing dependency {dep}')

            packages[name] = Package(**package)
            all_packages.append(name)

        # Sort the packages in dependency order
        graph = {k: v.dependencies for k, v in packages.items()}
        order: list[str] = list(TopologicalSorter(graph).static_order())
        packages: dict[str, Package] = {k: packages[k] for k in order}

        # Update dependencies to point to the actual package now
        # that the Package objecs are built
        for package in packages.values():
            package.dependencies = [packages[dep] for dep in package.dependencies]

        # Add implicit dependencies
        for package in packages.values():
            deps = sum([dep.dependencies for dep in package.dependencies], [])
            for dep in deps:
                if dep not in package.dependencies:
                    package.dependencies = [dep] + package.dependencies

        # Add influential files from dependencies. This ordering is
        # verify important, as before we had explicit dependencies
        # we filled in influential files in order top down from
        # the config
        add_influential: dict[str, list[str]] = {}
        for name, package in packages.items():
            add_influential[name] = sum([dep.influential for dep in package.dependencies], [])
        for name, files in add_influential.items():
            packages[name].influential = add_influential[name] + packages[name].influential

        # Sort the influential files so that the hash is
        # independent of the order that they are defined
        if not old_influential:
            for package in packages.values():
                package.influential = sorted(list(set(package.influential)))

        # Get the hashes for all influential files
        files = list(itertools.chain(*[p.influential for p in packages.values()]))
        all_hashes = Versioner.get_influential_hashes(files, commit)

        # Build the hashes from the influential files. Needs to
        # be done later after dependencies are provessed
        for package in packages.values():
            package.hash = Versioner.get_package_hash(package, all_hashes)

            # Without Versioner version management, the version
            # is just the hash
            if old_versions:
                package.version = package.hash
                package.full_version = package.hash
            # With Versioner version management, the version
            # comes from the named version in versioner.yaml
            else:
                if package.is_app:
                    package.full_version = package.hash
                    package.version = package.hash
                package.full_version = package.version
                if package.build_number is not None:
                    package.full_version += f'_{package.build_number}'

            package.apptainer = Versioner.get_apptainer_package(package)
            package.conda = Versioner.get_conda_package(package, commit)

        return packages

    @staticmethod
    def get_apptainer_package(package: Package) -> Union[ApptainerPackage, None]:
        """
        Builds the ApptainerPackage information for a given Package.
        """
        if 'apptainer' not in package.config:
            return None

        apptainer = package.config['apptainer']
        if apptainer is None:
            apptainer = {}
        package_name = package.name.lower()

        # The def file isn't named after the app, it's always "app"
        def_package = 'app' if package.is_app else package_name

        name_base = package_name
        if not name_base.startswith('moose-') and not package.is_app:
            name_base = f'moose-{name_base}'
        name_suffix = platform.machine()
        name = f'{name_base}-{name_suffix}'

        from_name = apptainer.get('from')
        if from_name:
            if from_name not in [dep.name for dep in package.dependencies]:
                Versioner().error(f'apptainer.from for {package.name} '
                                   'is not a dependency')

        values = {'name': name,
                  'from_name': from_name,
                  'name_base': name_base,
                  'name_suffix': name_suffix,
                  'tag': package.full_version,
                  'uri': f'{name}:{package.full_version}',
                  'def_path': os.path.abspath(os.path.join(MOOSE_DIR,
                                                           f'apptainer/{def_package}.def'))}

        return ApptainerPackage(**values)

    @staticmethod
    def get_conda_package(package: Package,
                          commit: str) -> Union[CondaPackage, None]:
        """
        Builds the CondaPackage information for the given Package
        at the given commit.
        """
        conda_dir = package.config.get('conda')
        if not conda_dir:
            for file in package.influential:
                if file.endswith(f'{package.name}/meta.yaml'):
                    conda_dir = os.path.dirname(file)
                    break

        if not conda_dir:
            return None

        meta_path = Versioner.conda_meta_path(conda_dir)
        meta_contents = Versioner.git_file(meta_path, commit)
        meta = yaml.safe_load(Versioner.render_jinja(meta_contents))

        # Parse from the meta.yaml file
        values = {'name': meta['package']['name'],
                  'version': str(meta['package']['version']),
                  'build_string': meta['build'].get('string', None),
                  'conda_dir': conda_dir,
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
    def conda_meta_path(conda_dir: os.PathLike) -> str:
        """
        Path to the conda meta file given a conda directory
        """
        return os.path.join(conda_dir, 'meta.yaml')

    @staticmethod
    def conda_build_config_path(conda_dir: os.PathLike) -> str:
        """
        Path to the conda build config file given a conda directory
        """
        return os.path.join(conda_dir, 'conda_build_config.yaml')

    @staticmethod
    def conda_influential(conda_dir: os.PathLike) -> list[str]:
        """
        Path to the assumed influential files for a conda package
        """
        return [Versioner.conda_meta_path(conda_dir),
                Versioner.conda_build_config_path(conda_dir)]

    @staticmethod
    def get_template_config() -> TemplateConfig:
        """
        Gets the configuration for templating
        """
        return TemplateConfig(prefix='__VERSIONER_',
                              suffix='__',
                              variables=['version', 'build_number'])

    @staticmethod
    def get_template_replacements(package: Package) -> list[Tuple[str, str]]:
        """
        Gets the variables that would be replaced in a
        template for the given package
        """
        config = Versioner.get_template_config()
        replacements = []
        for p in [package] + package.dependencies:
            name = p.name.replace('-', '_').upper()
            for var in config.variables:
                template_variable = f'{config.prefix}{name}_{var.upper()}{config.suffix}'
                replacement = str(getattr(p, var))
                replacements.append((template_variable, replacement))
        return replacements

    @staticmethod
    def augment_template(package: Package,
                         file: os.PathLike) -> str:
        """
        Augments a templated file, replacing all
        __VERSIONER_*__ entries
        """
        config = Versioner.get_template_config()

        path = os.path.join(MOOSE_DIR, file)
        with open(path, 'r', encoding="utf-8") as f:
            contents = f.read()

        # config = Versioner.get_template_config('HEAD')
        replacements = Versioner.get_template_replacements(package)

        # Do the replacements
        for from_val, to_val in replacements:
            contents = contents.replace(from_val, to_val)

        # We should no longer have any __VERSIONER_ stuff left,
        # otherwise we forgot a dependency
        search = re.search(r'{}.*{}'.format(config.prefix, config.suffix), contents)
        if search:
            Versioner.error(f'Unused template variable {search.group(0)} '
                            f'still exists in {file}')

        return contents

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
        parser.add_argument('-v', '--verify', nargs=1, metavar='base_ref', default=None,
                            help='Output version/build number hints against supplied base reference'
                            ' hash')
        parser.add_argument('--brief', action='store_true', default=False,
                            help='Output in brief form with brief (with --verify)')
        parser.add_argument('--build-templates', action='store_true',
                            help='Builds the templates')
        return parser.parse_args(argv)

    @staticmethod
    def check_args(args: argparse.Namespace) -> None:
        """ checks command line options """
        actions = ['json', 'yaml', 'summary', 'verify', 'build_templates']
        for action in actions:
            if getattr(args, action):
                for other_action in actions:
                    if action != other_action and getattr(args, other_action):
                        Versioner.error(f"Cannot use --{action} and "
                                        f"--{other_action} together")

        if args.verify and args.verify == 'HEAD':
            Versioner.error('You cannot verify against HEAD. You must choose a hash'
                            ' (preferably something like upstream/master)')
        if args.brief and not args.verify:
            Versioner.error('Cannot use --brief without --verify')

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
                raise ValueError(f'Supplied path {file} is not in {MOOSE_DIR}')
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
                       hash=str(git_hash[0:7]))

    @staticmethod
    def git_rev_parse(ref: str, directory: os.PathLike = MOOSE_DIR) -> str:
        """
        Calls `git rev-parse` with the given reference
        """
        cmd = ['git', 'rev-parse', ref]
        try:
            return subprocess.check_output(cmd, text=True, cwd=directory).rstrip()
        except subprocess.CalledProcessError as e:
            if e.returncode == 128:
                raise ValueError(f'Reference {ref} is not valid in repo {directory}') from e
            raise

    @staticmethod
    def versioner_yaml_path(commit: str) -> str:
        """
        Get the path to the Versioner configuration file at
        the given commit

        Needs to exist due to the file change from
        module_hash.yaml -> versioner.yaml
        """
        changed_commit = '2bd844dc5d4de47238eab94a3a718e9714592de1'
        use_module_hash = not Versioner.after_commit(changed_commit, commit)
        path = 'module_hash.yaml' if use_module_hash else 'versioner.yaml'
        return os.path.abspath(os.path.join(MOOSE_DIR, 'scripts', path))

    @staticmethod
    def using_old_influential(commit: str) -> bool:
        """
        Whether or not the old influential method is used
        at the given commit, in which the influential files
        are aggregatedas worked through the configuration file
        """
        changed_commit = '0e0785ee8a25742715b49bc26871117b788e7190'
        return not Versioner.after_commit(changed_commit, commit)

    @staticmethod
    def using_managed_versions(commit: str) -> bool:
        """
        Whether or not
        """
        changed_commit = '9da58ad93351b19cb23d850361f00ab98db3330b'
        return Versioner.git_ancestor(changed_commit, commit)

    @staticmethod
    def load_versioner_yaml(commit: str) -> dict:
        """
        Loads the versioner.yaml configuration file
        """
        path = Versioner.versioner_yaml_path(commit)
        try:
            contents = yaml.safe_load(Versioner.git_file(path, commit))
        except FileNotFoundError:
            Versioner.error(f'{path} not found')
        except yaml.scanner.ScannerError:
            Versioner.error(f'{path} parsing error')
        return contents

    @staticmethod
    def get_influential_hashes(files: list[str],
                               commit: str) -> dict[str, str]:
        """
        Gets the commit hashes for influential files for each of
        the given files at the given commit.

        The resulting dict maps file -> hash.
        """
        files = list(set(files))
        with concurrent.futures.ThreadPoolExecutor() as executor:
            get_hash = lambda file: Versioner.git_hash(file, commit)
            hashes_list = list(executor.map(get_hash, files))
        hashes = {}
        for i in range(len(files)):
            hashes[files[i]] = hashes_list[i]
        return hashes

    @staticmethod
    def get_package_hash(package: Package,
                         hashes: dict[str, str]) -> str:
        """
        Gets the version hash for the given package
        """
        if package.is_app:
            return Versioner.get_app_info().hash

        hashes = [hashes[f] for f in package.influential]
        combined = ''.join(hashes).encode('utf-8')
        return hashlib.md5(combined).hexdigest()[:7]

if __name__ == '__main__':
    result = Versioner().output_cli(sys.argv[1:])
    if result:
        print(result)
