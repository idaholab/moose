#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import unittest
import sys
import mooseutils
import subprocess
import copy
from mock import patch

MOOSE_DIR = mooseutils.git_root_dir()
sys.path.insert(0, os.path.join(MOOSE_DIR, 'scripts'))

from premake import PreMake

# Have a gold for conda list for configs that don't have
# conda available so that we can still test this
gold_conda_list = [{
    "base_url": "https://conda.software.inl.gov/public",
    "build_number": 0,
    "build_string": "build_0",
    "channel": "public",
    "dist_name": "moose-dev-2024.02.19-build_0",
    "name": "moose-dev",
    "platform": "osx-arm64",
    "version": "2024.02.19"
    },
    {
    "base_url": "https://conda.software.inl.gov/public",
    "build_number": 0,
    "build_string": "build_0",
    "channel": "public",
    "dist_name": "moose-libmesh-2024.01.23-build_0",
    "name": "moose-libmesh",
    "platform": "osx-arm64",
    "version": "2024.01.23"
},
{
    "base_url": "https://conda.software.inl.gov/public",
    "build_number": 8,
    "build_string": "build_8",
    "channel": "public",
    "dist_name": "moose-libmesh-vtk-9.2.6-build_8",
    "name": "moose-libmesh-vtk",
    "platform": "osx-arm64",
    "version": "9.2.6"
},
{
    "base_url": "https://conda.software.inl.gov/public",
    "build_number": 15,
    "build_string": "build_15",
    "channel": "public",
    "dist_name": "moose-mpich-4.0.2-build_15",
    "name": "moose-mpich",
    "platform": "osx-arm64",
    "version": "4.0.2"
},
{
    "base_url": "https://conda.software.inl.gov/public",
    "build_number": 0,
    "build_string": "hb4c3223_0",
    "channel": "public",
    "dist_name": "moose-peacock-2023.11.29-hb4c3223_0",
    "name": "moose-peacock",
    "platform": "osx-arm64",
    "version": "2023.11.29"
},
{
    "base_url": "https://conda.software.inl.gov/public",
    "build_number": 0,
    "build_string": "build_0",
    "channel": "public",
    "dist_name": "moose-petsc-3.20.3-build_0",
    "name": "moose-petsc",
    "platform": "osx-arm64",
    "version": "3.20.3"
},
{
    "base_url": "https://conda.software.inl.gov/public",
    "build_number": 0,
    "build_string": "he4b30e4_0",
    "channel": "public",
    "dist_name": "moose-tools-2023.12.20-he4b30e4_0",
    "name": "moose-tools",
    "platform": "osx-arm64",
    "version": "2023.12.20"
},
{
    "base_url": "https://conda.software.inl.gov/public",
    "build_number": 0,
    "build_string": "build_0",
    "channel": "public",
    "dist_name": "moose-wasp-2024.02.19-build_0",
    "name": "moose-wasp",
    "platform": "osx-arm64",
    "version": "2024.02.19"
}]

# Have a gold for the apptainer environment so that we can
# test it without being in apptainer
gold_apptainer_tag = '561161b'
gold_apptainer_library = 'moose-dev'
gold_apptainer_suffix = '-openmpi'
gold_apptainer_arch = 'x86_64'
gold_apptainer_env = {'TAG': gold_apptainer_tag,
                      'VERSION': gold_apptainer_tag,
                      'LIBRARY': gold_apptainer_library,
                      'NAME': f'{gold_apptainer_library}{gold_apptainer_suffix}-{gold_apptainer_arch}',
                      'NAME_SUMMARY': f'{gold_apptainer_library}{gold_apptainer_suffix}-{gold_apptainer_arch}:{gold_apptainer_tag}'}

class Test(unittest.TestCase):
    def testCondaList(self):
        # Can only test this if we actually have conda
        if os.environ.get('CONDA_PREFIX'):
            conda_list = PreMake.condaList()

            # Run our own conda list to make sure it all looks right
            cmd = ['conda', 'list']
            out = subprocess.check_output(cmd)
            result = out.decode('utf-8').splitlines()
            for line in result:
                # Skip comments
                if line.startswith('#'):
                    continue

                # Load this line; should be split by whitespace
                line_split = line.split()
                name = line_split[0]
                version = line_split[1]
                build_string = line_split[2]

                # Find it in the list
                found = False
                for gold_entry in conda_list:
                    if gold_entry.get('name') == name:
                        found = True
                        self.assertEqual(version, gold_entry.get('version'))
                        self.assertEqual(build_string, gold_entry.get('build_string'))
                self.assertTrue(found)

    @patch.object(PreMake, "condaList")
    def testGetCondaEnv(self, mock_conda_list):
        # So that we can still test this on systems that don't have conda
        mock_conda_list.return_value = gold_conda_list
        CONDA_PREFIX = os.environ.get('CONDA_PREFIX')
        if CONDA_PREFIX is None:
            os.environ['CONDA_PREFIX'] = 'foo'

        # Make sure the map does its thing
        conda_env = PreMake.getCondaEnv()
        self.assertEqual(len(conda_env), len(gold_conda_list))
        for entry in gold_conda_list:
            self.assertIn(entry.get('name'), conda_env)
            conda_env_entry = conda_env.get(entry.get('name'))
            self.assertEqual(conda_env_entry['version'], entry['version'])
            self.assertEqual(conda_env_entry['build_string'], entry['build_string'])

        # Test not having conda
        del os.environ['CONDA_PREFIX']
        self.assertEqual(PreMake.getCondaEnv(), None)

        # Set this back once we're done
        os.environ['CONDA_PREFIX'] = CONDA_PREFIX

    @patch.object(PreMake, "condaList")
    def testCheckCondaPackage(self, mock_conda_list):
        # So that we can still test this on systems that don't have conda
        mock_conda_list.return_value = gold_conda_list
        CONDA_PREFIX = os.environ.get('CONDA_PREFIX')
        if CONDA_PREFIX is None:
            os.environ['CONDA_PREFIX'] = 'foo'

        pre_make = PreMake()
        package = 'moose-dev'
        version = copy.copy(pre_make.conda_env[package]['version'])
        current_version = copy.copy(pre_make.versioner_meta[package]['conda']['version'])
        build_number = copy.copy(pre_make.conda_env[package]['build_number'])

        # Same version and build
        pre_make = PreMake()
        pre_make.conda_env[package]['version'] = current_version
        pre_make._checkCondaPackage(package)

        # Different version
        pre_make = PreMake()
        pre_make.conda_env[package]['version'] = 'foo'
        with self.assertRaises(PreMake.CondaVersionMismatch) as e:
            pre_make._checkCondaPackage(package)
        self.assertEqual(e.exception.package, package)
        self.assertEqual(e.exception.version, 'foo')
        self.assertEqual(e.exception.required_version, current_version)
        self.assertEqual(e.exception.build, build_number)
        self.assertEqual(e.exception.required_build, build_number)

        # Different build number
        pre_make = PreMake()
        different_build = 100
        pre_make.conda_env[package]['version'] = version
        pre_make.conda_env[package]['build_number'] = different_build
        with self.assertRaises(PreMake.CondaVersionMismatch) as e:
            pre_make._checkCondaPackage(package)
        self.assertEqual(e.exception.package, package)
        self.assertEqual(e.exception.version, version)
        self.assertEqual(e.exception.required_version, current_version)
        self.assertEqual(e.exception.build, different_build)
        self.assertEqual(e.exception.required_build, build_number)

        # Set this back once we're done
        os.environ['CONDA_PREFIX'] = CONDA_PREFIX

    def testGetApptainerEnv(self):
        prefix = 'MOOSE_APPTAINER_GENERATOR_'
        apptainer_env = {}
        keys = list(gold_apptainer_env.keys())
        for key in keys:
            entry = os.environ.get(f'{prefix}{key}')
            if entry:
                apptainer_env[key] = entry

        # If we have one of the keys, it means we're in an apptainer environment
        # and we should have all of them
        if os.environ.get(f'{prefix}{keys[0]}') is not None:
            self.assertEqual(len(apptainer_env), len(keys))
        # Otherwise, we should expect none
        else:
            self.assertEqual(len(apptainer_env), 0)

        if len(apptainer_env) == 0:
            apptainer_env = None
        self.assertEqual(apptainer_env, PreMake.getApptainerEnv())

    @patch.object(PreMake, "getApptainerEnv")
    def testCheckApptainer(self, mock_get_apptainer_env):
        # So that we can still test this on systems outside of apptainer
        mock_get_apptainer_env.return_value = gold_apptainer_env
        library = gold_apptainer_library

        # The current apptainer info for moose-dev
        pre_make = PreMake()
        apptainer_meta = pre_make.versioner_meta[library]['apptainer']
        tag = apptainer_meta['tag']

        # Same version
        pre_make = PreMake()
        pre_make.apptainer_env['TAG'] = tag
        pre_make.apptainer_env['VERSION'] = tag
        pre_make._checkApptainer()

        # Different version
        different_tag = 'abcd123'
        pre_make = PreMake()
        pre_make.apptainer_env['TAG'] = different_tag
        pre_make.apptainer_env['VERSION'] = different_tag
        with self.assertRaises(PreMake.ApptainerVersionMismatch) as e:
            pre_make._checkApptainer()
        self.assertEqual(e.exception.name, gold_apptainer_env['NAME'])
        self.assertEqual(e.exception.name_base, apptainer_meta['name_base'])
        self.assertEqual(e.exception.current_version, different_tag)
        self.assertEqual(e.exception.required_version, tag)
        self.assertIn(f'oras://harbor.hpc.inl.gov/{library}/{gold_apptainer_env["NAME"]}:{tag}', str(e.exception))

        # Message with a module instead
        container_module_name_before = os.environ.get('CONTIANER_MODULE_NAME')
        container_module_name = 'moose-dev-openmpi-foo'
        os.environ['CONTAINER_MODULE_NAME'] = container_module_name
        with self.assertRaises(PreMake.ApptainerVersionMismatch) as e:
            pre_make._checkApptainer()
            self.assertIn(f'{container_module_name}/{tag}', str(e.exception))

if __name__ == '__main__':
    unittest.main(verbosity=2, buffer=True)
