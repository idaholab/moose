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
import json
import tempfile
from mooseutils.jsondiff import JSONDiffer

class TestJSONDiff(unittest.TestCase):
    """
    Test that the size function returns something.
    """
    def setUp(self):
        data0 = json.loads('{"foo":["foz", 1, 1.0, 1.1e-1], "bar":["baz", null, 1.0, 1.0]}')
        self._tmpfile0 = tempfile.mkstemp(text=True)[-1]
        with open(self._tmpfile0, 'w', encoding='utf-8') as fid:
            json.dump(data0, fid)

        data1 = json.loads('{"foo":["foz", 1, 1.0, 1.1e-1], "bar":["baz", null, 1.1, 1.001]}')
        self._tmpfile1 = tempfile.mkstemp(text=True)[-1]
        with open(self._tmpfile1, 'w', encoding='utf-8') as fid:
            json.dump(data1, fid)

    def tearDown(self):
        os.remove(self._tmpfile0)
        os.remove(self._tmpfile1)

    def testFail(self):
        obj = JSONDiffer(self._tmpfile0, self._tmpfile1)
        self.assertEqual("    Value of root['bar'][2] changed from 1.0 to 1.1.\n    Value of root['bar'][3] changed from 1.0 to 1.001.", obj.message())

    def testFailWithRelativeError(self):
        obj = JSONDiffer(self._tmpfile0, self._tmpfile1, relative_error=1e-2)
        self.assertEqual("    Value of root['bar'][2] changed from 1.0 to 1.1.", obj.message())

    def testPassWithRelativeError(self):
        obj = JSONDiffer(self._tmpfile0, self._tmpfile1, relative_error=1.1e-1)
        self.assertEqual("    Files are the same", obj.message())

    def testFailWithSkipKeys(self):
        obj = JSONDiffer(self._tmpfile0, self._tmpfile1, skip_keys=['foo'])
        self.assertEqual("    Value of root['bar'][2] changed from 1.0 to 1.1.\n    Value of root['bar'][3] changed from 1.0 to 1.001.", obj.message())

    def testPassWithSkipKeys(self):
        obj = JSONDiffer(self._tmpfile0, self._tmpfile1, skip_keys=['bar'])
        self.assertEqual("    Files are the same", obj.message())

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2, buffer=True, exit=False)
