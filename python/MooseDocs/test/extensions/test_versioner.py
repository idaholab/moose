#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import unittest
import logging
import os
import sys
from MooseDocs.test import MooseDocsTestCase
from MooseDocs.extensions import core, command, versioner
from MooseDocs import base, MOOSE_DIR
logging.basicConfig()

sys.path.append(os.path.join(MOOSE_DIR, 'scripts'))
from versioner import *

class TestTemplate(MooseDocsTestCase):
    EXTENSIONS = [core, command, versioner]

    def setUp(self):
      self.version_meta = Versioner().version_meta()

    def setupExtension(self, ext):
        if ext == versioner:
            return dict(active=True)

    def _testRender(self, cmd, versioner_keys):
        """Helper for testing the render stage given a command
        and the keys to look up in the versioner meta"""
        for package in versioner.TRACKING_LIBRARIES:
            text = f"The version is [!versioner!{cmd} package={package}]"
            _, res = self.execute(text, renderer=base.MaterializeRenderer())
            self.assertHTMLTag(res, 'div', size=1)
            version = self.version_meta[package]
            for key in versioner_keys:
                version = version.get(key)
            self.assertEqual(res(0).text(), f'The version is {version}')

    def testVersionRender(self):
        """Test [!versioner!version package=<package>]

        This pulls from the Versioner meta in:
        Versioner().version_meta()[<package>]['hash']"""
        self._testRender('version', ['hash'])

    def testCondaVersionRender(self):
        """Test [!versioner!conda_version package=<package>],

        This pulls from the Versioner meta in:
        Versioner().version_meta()[<package>]['conda']['version_and_build']"""
        self._testRender('conda_version', ['conda', 'version_and_build'])

    def testCodeRender(self):
        """Test inline replacement within code blocks"""
        for package in versioner.TRACKING_LIBRARIES:
            # Application, doesn't have a conda version
            # Several support libraries also have no conda version
            has_conda_version = package not in ['app',
                                                'libmesh-vtk',
                                                'peacock',
                                                'pprof',
                                                'pyhit',
                                                'seacas']

            # Change something like "moose-dev" -> "MOOSE_DEV"
            package_inline = package.upper().replace('-', '_')

            text = '!versioner! code\n'
            if has_conda_version:
                text += f'conda install {package}=__VERSIONER_CONDA_VERSION_{package_inline}__\n'
            text += f'module load {package}/__VERSIONER_VERSION_{package_inline}__\n'
            text += '!versioner-end!\n'

            _, res = self.execute(text, renderer=base.MaterializeRenderer())
            self.assertHTMLTag(res, 'div', size=1)

            version = self.version_meta[package]['hash']
            if has_conda_version:
                conda_version = self.version_meta[package]['conda']['version_and_build']

            expected_text = ''
            if has_conda_version:
                expected_text = f'conda install {package}={conda_version}\n'
            expected_text += f'module load {package}/{version}\n'
            self.assertEqual(res(0).text(), expected_text)

if __name__ == '__main__':
    unittest.main(verbosity=2)
