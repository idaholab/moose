#!/usr/bin/env python
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
from TestHarnessTestCase import TestHarnessTestCase

class TestJSONDump(TestHarnessTestCase):
    FILENAME = os.path.join(os.getenv('MOOSE_DIR'), 'test', 'testoutput.json')

    def setUp(self):
        if os.path.isfile(TestJSONDump.FILENAME):
            os.remove(TestJSONDump.FILENAME)

    def testJSON(self):
        self.runTests('--json')
        self.assertTrue(os.path.isfile(TestJSONDump.FILENAME))

if __name__ == '__main__':
    unittest.main(verbosity=2)
