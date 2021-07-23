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
import copy
import platform
import subprocess
from MooseDocs.commands import check

@unittest.skipIf(mooseutils.git_version() < (2,11,4), "Git version must at least 2.11.4")
class TestCheckScript(unittest.TestCase):
    def testCheck(self, *args):

        # Test that script runs from within the containing directory
        cmd = ['python', 'moosedocs.py', 'check', '--config', 'sqa_reports.yml']
        cwd = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', '..', 'test', 'doc'))
        try:
            mooseutils.check_output(cmd, cwd=cwd, check=False, timeout=5)
        except subprocess.TimeoutExpired as ex:
            out = str(ex.stdout)
        self.assertIn('MOOSEAPP REPORT(S):', out)

        # Test that script runs from within the containing directory, without MOOSE_DIR
        env = copy.copy(os.environ)
        env.pop('MOOSE_DIR', None)
        try:
            out = mooseutils.check_output(cmd, cwd=cwd, env=env, check=False, timeout=5)
        except subprocess.TimeoutExpired as ex:
            out = str(ex.stdout)
        self.assertIn('MOOSEAPP REPORT(S):', out)

        # Test that script runs from without the containing directory
        cmd = ['python', 'test/doc/moosedocs.py', 'check', '--config', 'sqa_reports.yml']
        cwd = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', '..'))
        try:
            out = mooseutils.check_output(cmd, cwd=cwd, check=False, timeout=5)
        except subprocess.TimeoutExpired as ex:
            out = str(ex.stdout)
        self.assertIn('MOOSEAPP REPORT(S):', out)

        # Test that script runs from without the containing directory and without MOOSE_DIR
        env = copy.copy(os.environ)
        env.pop('MOOSE_DIR', None)
        try:
            out = mooseutils.check_output(cmd, cwd=cwd, env=env, timeout=5)
        except subprocess.TimeoutExpired as ex:
            out = str(ex.stdout)
        self.assertIn('MOOSEAPP REPORT(S):', out)

@unittest.skipIf(mooseutils.git_version() < (2,11,4), "Git version must at least 2.11.4")
class TestCheck(unittest.TestCase):
    def setUp(self):
        # Change to the test/doc directory
        self._working_dir = os.getcwd()
        moose_test_doc_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', '..', 'test', 'doc'))
        os.chdir(moose_test_doc_dir)

    def tearDown(self):
        # Restore the working directory
        os.chdir(self._working_dir)

    def testCheck(self):

        # Create command-line arguments in
        opt = types.SimpleNamespace(config='sqa_reports.yml', reports=['app'], dump=None,
                                    app_reports=None, req_reports=None,
                                    generate=['MooseTestApp'], show_warnings=False)

        # --reports
        opt.reports = ['app']
        opt.config = 'sqa_reports.yml'
        with mock.patch('sys.stdout', new=io.StringIO()) as stdout:
            status = check.main(opt)
        self.assertIn('MOOSEAPP REPORT(S)', stdout.getvalue())
        self.assertNotIn('DOCUMENT REPORT(S)', stdout.getvalue())
        self.assertNotIn('REQUIREMENT REPORT(S)', stdout.getvalue())

        opt.reports = ['doc', 'req']
        with mock.patch('sys.stdout', new=io.StringIO()) as stdout:
            status = check.main(opt)
        self.assertNotIn('MOOSEAPP REPORT(S)', stdout.getvalue())
        self.assertIn('DOCUMENT REPORT(S)', stdout.getvalue())
        self.assertIn('REQUIREMENT REPORT(S)', stdout.getvalue())

        # --app-reports
        opt.reports = ['app']
        opt.app_reports = ['not-a-value']
        with mock.patch('sys.stdout', new=io.StringIO()) as stdout:
            status = check.main(opt)
        self.assertNotIn('MOOSEAPP REPORT(S)', stdout.getvalue())
        self.assertNotIn('DOCUMENT REPORT(S)', stdout.getvalue())
        self.assertNotIn('REQUIREMENT REPORT(S)', stdout.getvalue())

        opt.app_reports = ['framework']
        with mock.patch('sys.stdout', new=io.StringIO()) as stdout:
            status = check.main(opt)
        self.assertIn('MOOSEAPP REPORT(S)', stdout.getvalue())
        self.assertNotIn('DOCUMENT REPORT(S)', stdout.getvalue())
        self.assertNotIn('REQUIREMENT REPORT(S)', stdout.getvalue())

        # --req-reports
        opt.reports = ['req']
        opt.req_reports = ['not-a-value']
        with mock.patch('sys.stdout', new=io.StringIO()) as stdout:
            status = check.main(opt)
        self.assertNotIn('MOOSEAPP REPORT(S)', stdout.getvalue())
        self.assertNotIn('DOCUMENT REPORT(S)', stdout.getvalue())
        self.assertNotIn('REQUIREMENT REPORT(S)', stdout.getvalue())

        opt.req_reports = ['moose_test']
        with mock.patch('sys.stdout', new=io.StringIO()) as stdout:
            status = check.main(opt)
        self.assertNotIn('MOOSEAPP REPORT(S)', stdout.getvalue())
        self.assertNotIn('DOCUMENT REPORT(S)', stdout.getvalue())
        self.assertIn('REQUIREMENT REPORT(S)', stdout.getvalue())


if __name__ == '__main__':
    unittest.main(verbosity=2)
