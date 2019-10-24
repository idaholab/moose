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
from peacock.utils.FileCache import FileCache
from peacock.utils import Testing
from PyQt5 import QtWidgets

class Tests(Testing.PeacockTester):
    qapp = QtWidgets.QApplication([])

    def checkFileCache(self, fc, dirty=True, val={}, path_data={}):
        self.assertEqual(fc.dirty, dirty)
        self.assertEqual(fc.val, val)
        self.assertEqual(fc.path_data, path_data)

    @unittest.skip("Needs update for Python 3")
    def testBasic(self):
        key = "test_FileCache"
        obj = {"foo": "bar"}

        FileCache.clearAll(key)
        fc = FileCache(key, "/no_exist", 1)
        self.checkFileCache(fc)
        self.assertEqual(fc.no_exist, True)

        ret = fc.add(obj)
        self.assertEqual(ret, False)
        self.assertEqual(fc.dirty, True)

        exe_path = Testing.find_moose_test_exe()
        fc = FileCache(key, exe_path, 1)
        self.checkFileCache(fc)
        val = fc.read()
        self.assertEqual(val, None)

        ret = fc.add(obj)
        self.assertEqual(ret, True)
        self.assertEqual(fc.dirty, False)
        ret = fc.add(obj)
        self.assertEqual(ret, False)
        self.assertEqual(fc.dirty, False)
        val = fc.read()
        self.assertEqual(val, obj)
        self.assertEqual(fc.dirty, False)

        fc = FileCache(key, exe_path, 1)
        self.assertEqual(fc.dirty, False)
        # different data version
        fc = FileCache(key, exe_path, 2)
        self.assertEqual(fc.dirty, True)

        FileCache.clearAll(key)

if __name__ == '__main__':
    Testing.run_tests()
