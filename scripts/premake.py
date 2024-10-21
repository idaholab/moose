#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import subprocess, os, platform, re, sys, traceback
from datetime import date

class PreMake:
    def __init__(self):
        self.conda_env = self.getCondaEnv()
        self.apptainer_env = self.getApptainerEnv()

        try:
            from versioner import Versioner
        except Exception as e:
            warning = 'Failed to initialize PreMake for version checking; '
            warning += 'this may be ignored but may suggest an environment issue.'
            warning += f'\n\n{traceback.format_exc()}'
            self.warn(warning)

        self.versioner_meta = Versioner().version_meta()

    @staticmethod
    def printColored(msg, color, **kwargs):
        """
        Prints a colored message
        """
        color_vals = {'red': 31, 'green': 32, 'yellow': 33}
        if color not in color_vals:
            raise Exception('Unknown prefix color {}'.format(color))
        print(f'\033[{color_vals[color]}m{msg}\033[0m', **kwargs)

    @staticmethod
    def warn(msg):
        """
        Prints a warning in yellow
        """
        PreMake.printColored(f'WARNING: {msg}', 'yellow', file=sys.stderr)

    @staticmethod
    def condaList():
        """
        Gets the conda package list in JSON form

        Separated out on purpose so that we can mock it in unit tests,
        running tests even when conda doesn't exist.
        """
        cmd = ['conda', 'list', '--json']
        process = subprocess.run(cmd, stdout=subprocess.PIPE,
                                    stderr=subprocess.PIPE,
                                    encoding='utf-8')
        if process.returncode != 0:
            raise Exception(f"Failed to get conda env: '{' '.join(cmd)}'\n{process.stdout}")

        try:
            import json
        except Exception as e:
            raise Exception(f"Failed to import json for conda list") from e

        return json.loads(process.stdout)

    @staticmethod
    def getCondaEnv():
        """
        Gets the conda package environment in the form of a dict of
        package name to information about that package.

        The keys that we require to exist for each package entry:
        version, build_string

        Will return None if conda is not available.
        """
        conda_env = None

        if os.environ.get('CONDA_PREFIX') is not None:
            conda_list = PreMake.condaList()

            conda_env = {}
            for entry in conda_list:
                conda_env[entry['name']] = entry

                # Make sure the keys exist that we expect to be there
                for key in ['base_url', 'build_string', 'version']:
                    if key not in entry:
                        raise Exception(f'Missing expected key {key} in conda package list')

        return conda_env

    @staticmethod
    def getApptainerEnv():
        """
        Gets the apptainer generator generated apptainer environment
        information if it exists.
        """
        prefix = 'MOOSE_APPTAINER_GENERATOR_'
        apptainer_env = {}
        for key, value in os.environ.items():
            if key.startswith(prefix):
                apptainer_env[key.replace(prefix, '')] = value
        return apptainer_env if apptainer_env else None

    class VersionMismatch(Exception):
        pass

    class CondaVersionMismatch(VersionMismatch):
        """
        Exception that denotes the mismatch of a version or build
        in a conda packages
        """
        def __init__(self, package, version, build, required_version, required_build, msg=None):
            self.package = package
            self.version = version
            self.required_version = required_version
            self.build = build
            self.required_build = required_build

            current = f'{version}'
            required = f'{required_version}'
            # Same version, different build, include build string
            if version == required_version:
                current += f'={build}'
                required += f'={required_build}'

            full_message = f"Conda package '{package}' is currently at version "
            full_message += f"'{current}' and the required version is '{required}'.\n"

            # If the version is a date and the current date is newer than the
            # required date, it's likely that MOOSE needs to be updated
            show_conda_install = True
            version_date = self.parseVersionDate(version)
            required_version_date = self.parseVersionDate(required_version)
            if version_date is not None and required_version_date is not None:
                if required_version_date < version_date:
                    show_conda_install = False
                    full_message += f"The installed version of '{package}' is newer than the required version.\n"
                    full_message += "It is likely that you need to update moose."

            if show_conda_install:
                full_message += "The correct version can be installed via:\n"
                full_message += f"    conda install {package}={required}"

            if msg:
                full_message += f'\n{msg}'
            super().__init__(full_message)

        @staticmethod
        def parseVersionDate(version: str):
            """
            Helper for parsing a date from a version if the version is a date
            """
            version_date_re = re.search(r'(\d{4}).(\d{2}).(\d{2})', version)
            if version_date_re is None:
                return None
            return date(int(version_date_re.group(1)),
                        int(version_date_re.group(2)),
                        int(version_date_re.group(3)))

    class ApptainerVersionMismatch(VersionMismatch):
        """
        Exception that denotes the mismatch of an apptainer
        container version
        """
        def __init__(self, name, name_base, current_version, required_version):
            self.name = name
            self.name_base = name_base
            self.current_version = current_version
            self.required_version = required_version

            message = f'Container {name} is currently at version {current_version} '
            message += f'and the required version is {required_version}.\n'
            message += f'Before updating the container, make sure that your version '
            message += f'of MOOSE is up to date.\n\n'

            # Using a loaded module on INL HPC
            CONTAINER_MODULE_NAME = os.environ.get('CONTAINER_MODULE_NAME')
            if CONTAINER_MODULE_NAME == name.replace(f'-{platform.machine()}', ''):
                message += 'You can obtain the correct container via the module '
                message += f'{CONTAINER_MODULE_NAME}/{required_version}.'
            # Not using a loaded module on INL HPC
            else:
                message += 'You can obtain the correct container at '
                if name_base == 'moose-dev':
                    harbor = 'harbor.hpc.inl.gov'
                else:
                    harbor = 'mooseharbor.hpc.inl.gov'
                message += f'oras://{harbor}/{name_base}/{name}:{required_version}.'

            super().__init__(message)

    def check(self):
        """
        Checks for build issues
        """
        try:
            self._check()
        except self.VersionMismatch as e:
            self.warn(str(e))
        except Exception as e:
            warning = 'PreMake check failed; this may be ignored but may suggest an environment issue'
            warning += f'\n\n{traceback.format_exc()}'
            self.warn(warning)

    def _checkCondaPackage(self, package_name, versioner_name=None):
        """
        Internal method for checking if a given conda package is up to date
        """
        package = self.conda_env.get(package_name)
        if package:
            if versioner_name is None:
                versioner_name = package_name
            package_tuple = (package['version'], package['build_number'])
            version_tuple = (self.versioner_meta[versioner_name]['conda']['version'],
                             self.versioner_meta[versioner_name]['conda']['build'])
            if package_tuple != version_tuple:
                raise self.CondaVersionMismatch(package_name,
                                                *package_tuple,
                                                *version_tuple,
                                                version_tuple[1])

    def _checkApptainer(self):
        library = self.apptainer_env['LIBRARY']

        library_meta = self.versioner_meta.get(library)
        if not library_meta:
            return

        apptainer_meta = library_meta.get('apptainer')
        if not apptainer_meta:
            return

        required_version = apptainer_meta['tag']
        current_version = self.apptainer_env['VERSION']
        if required_version != current_version:
            name = self.apptainer_env['NAME']
            name_base = apptainer_meta['name_base']
            raise self.ApptainerVersionMismatch(name, name_base, current_version, required_version)

    def _check(self):
        """
        Internal check method
        """
        # We have apptainer available, check the version if we can
        if self.apptainer_env:
            self._checkApptainer()

        # We have conda available, check those environments
        if self.conda_env:
            self._checkCondaPackage('moose-dev')
            self._checkCondaPackage('moose-libmesh', 'libmesh')
            self._checkCondaPackage('moose-petsc', 'petsc')
            self._checkCondaPackage('moose-mpi', 'mpi')
            self._checkCondaPackage('moose-wasp', 'wasp')

if __name__ == '__main__':
    PreMake().check()
