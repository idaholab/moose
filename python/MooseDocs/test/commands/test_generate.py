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
import mock
import types
import io
import mooseutils
import moosesqa
from MooseDocs.commands import generate

@unittest.skipIf(mooseutils.git_version() < (2,11,4), "Git version must at least 2.11.4")
class TestGenerate(unittest.TestCase):
    def setUp(self):
        # Change to the test/doc directory
        self._working_dir = os.getcwd()
        moose_test_doc_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', '..', 'test', 'doc'))
        os.chdir(moose_test_doc_dir)

    def tearDown(self):
        # Restore the working directory
        os.chdir(self._working_dir)

    @mock.patch('MooseDocs.commands.generate._shouldCreateStub')
    @mock.patch('MooseDocs.commands.generate._writeFile')
    def testGenerate(self, writeFile, shouldCreateStub):

        # Store the filenames to be created
        filenames = list()
        writeFile.side_effect = lambda fn, *args: filenames.append(fn)

        # Create custom function for determining if a stub should be created
        shouldCreateStub.side_effect = self._shouldCreateStub

        # Run the generate command
        opt = types.SimpleNamespace(app_types=['MooseApp'], config='sqa_reports.yml')
        status = generate.main(opt)

        self.assertEqual(status, 0)
        self.assertEqual(len(filenames), 3)
        self.assertTrue(filenames[0].endswith('moose/framework/doc/content/syntax/Kernels/index.md'))
        self.assertTrue(filenames[1].endswith('moose/framework/doc/content/source/actions/AddKernelAction.md'))
        self.assertTrue(filenames[2].endswith('moose/framework/doc/content/source/kernels/Diffusion.md'))

    @staticmethod
    def _shouldCreateStub(report, n):
        # Test for stub on Action, Object, and Syntax
        if n.fullpath() in ('/Kernels/AddKernelAction', '/Kernels/Diffusion', '/Kernels'):
            return True
        return False

if __name__ == '__main__':
    unittest.main(verbosity=2)
