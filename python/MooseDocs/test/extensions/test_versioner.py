#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
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
      super().setUp()
      self.packages = Versioner().get_packages('HEAD')

    def setupExtension(self, ext):
        if ext == versioner:
            return dict(active=True)

    def _testRender(self, cmd, versioner_keys, conda=None):
        """Helper for testing the render stage given a command
        and the keys to look up in the versioner meta"""
        for name, package in self.packages.items():
            if conda == True and not package.conda:
                continue
            text = f"The version is [!versioner!{cmd} package={name}]"
            _, res = self.execute(text, renderer=base.MaterializeRenderer())
            self.assertHTMLTag(res, 'div', size=1)
            value = package
            for key in versioner_keys:
                value = getattr(value, key)
            self.assertEqual(res(0).text(), f'The version is {value}', f'package={name}')

    def testVersionRender(self):
        """Test [!versioner!version package=<package>]

        This pulls from the Versioner meta in:
        Versioner().get_packages(...)[<package>].full_version"""
        self._testRender('version', ['full_version'])

    def testCondaVersionRender(self):
        """Test [!versioner!conda_version package=<package>],

        This pulls from the Versioner meta in:
        Versioner().get_packages(...)[<package>].conda.install"""
        self._testRender('conda_version', ['conda', 'install'], conda=True)

    def testCodeRender(self):
        """Test inline replacement within code blocks"""
        for name, package in self.packages.items():
            # Change something like "moose-dev" -> "MOOSE_DEV"
            package_inline = name.upper().replace('-', '_')

            package = self.packages[name]
            text = '!versioner! code\n'
            if package.conda:
                text += f'conda install {name}=__VERSIONER_CONDA_VERSION_{package_inline}__\n'
            text += f'module load {name}/__VERSIONER_VERSION_{package_inline}__\n'
            text += '!versioner-end!\n'

            _, res = self.execute(text, renderer=base.MaterializeRenderer())
            self.assertHTMLTag(res, 'div', size=1)

            expected_text = ''
            if package.conda:
                expected_text = f'conda install {name}={package.conda.install}\n'
            expected_text += f'module load {name}/{package.full_version}\n'
            self.assertEqual(res(0).text(), expected_text)

if __name__ == '__main__':
    unittest.main(verbosity=2)
