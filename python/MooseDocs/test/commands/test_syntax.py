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
from MooseDocs.commands import syntax

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

    @mock.patch('mooseutils.colorText')
    def testDump(self, colorText):
        colorText.side_effect = lambda txt, *args, **kwargs: txt

        # Run the generate command
        opt = types.SimpleNamespace(config='sqa_reports.yml')
        with mock.patch('sys.stdout', new=io.StringIO()) as stdout:
            status = syntax.main(opt)
        self.assertIn('Adaptivity: /Adaptivity', stdout.getvalue())

if __name__ == '__main__':
    unittest.main(verbosity=2)
