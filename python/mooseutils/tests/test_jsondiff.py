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
        data0 = json.loads('["foo", {"bar":["baz", null, 1.0, 2]}]')
        self._tmpfile0 = tempfile.mkstemp(text=True)[-1]
        with open(self._tmpfile0, 'w', encoding='utf-8') as fid:
            json.dump(data0, fid)

        data1 = json.loads('["foo", {"bar":["baz", null, 2.0, 2]}]')
        self._tmpfile1 = tempfile.mkstemp(text=True)[-1]
        with open(self._tmpfile1, 'w', encoding='utf-8') as fid:
            json.dump(data1, fid)

    def tearDown(self):
        os.remove(self._tmpfile0)
        os.remove(self._tmpfile1)

    def test(self):
        obj = JSONDiffer(self._tmpfile0, self._tmpfile1, color=False)
        self.assertIn('-            2.0', obj.message())
        self.assertIn('+            1.0', obj.message())

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2, buffer=True, exit=False)
