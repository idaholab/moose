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
import mock
import logging
from moosesqa import SQAReport, SQAMooseAppReport

class TestSQAMooseAppReport(unittest.TestCase):

    def testExeError(self):
        kwargs = dict(exe_name='wrong',
                      exe_directory='test',
                      content_directory='framework/doc/content',
                      remove=['framework/doc/remove.yml', 'test/doc/remove.yml'])

        reporter = SQAMooseAppReport(**kwargs)
        with self.assertRaises(IOError):
            reporter.getReport()

if __name__ == '__main__':
    unittest.main(verbosity=2)
