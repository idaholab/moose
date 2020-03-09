#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from peacock.Input.JsonData import JsonData
from peacock.utils import Testing
from PyQt5 import QtWidgets

class Tests(Testing.PeacockTester):
    qapp = QtWidgets.QApplication([])

    def setUp(self):
        super(Tests, self).setUp()
        self.json_changed_count = 0
        self.test_path = "/Variables/*/InitialCondition"

    def check(self, y, exists):
        entry = y.findPath(self.test_path)
        if exists:
            self.assertNotEqual(entry, None)
            out = y.dumpNames()
            self.assertIn(self.test_path, out)
        else:
            self.assertEqual(entry, None)

    def testJsonData(self):
        y = JsonData()
        self.assertEqual(y.json_data, None)
        y.appChanged("No exist")
        self.assertEqual(y.json_data, None)
        path = Testing.find_moose_test_exe()
        y.appChanged(path)
        self.assertNotEqual(y.json_data, None)
        self.assertIn("blocks", y.json_data.keys())
        self.assertIn("global", y.json_data.keys())
        self.assertIn("Variables", y.json_data["blocks"].keys())

    def testPickle(self):
        y = JsonData()
        self.assertEqual(y.json_data, None)
        path = Testing.find_moose_test_exe()
        y.appChanged(path)
        self.assertNotEqual(y.json_data, None)
        p = y.toPickle()
        y2 = JsonData()
        self.assertEqual(y2.json_data, None)
        y2.fromPickle(p)
        self.assertEqual(y2.app_path, y.app_path)
        self.assertEqual(y2.json_data, y.json_data)

if __name__ == '__main__':
    Testing.run_tests()
