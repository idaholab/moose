#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from peacock.Input.CheckInputWidget import CheckInputWidget
from PyQt5.QtTest import QTest
from PyQt5.QtCore import Qt
from peacock.utils import Testing
from PyQt5 import QtWidgets

class Tests(Testing.PeacockTester):
    qapp = QtWidgets.QApplication([])

    def setUp(self):
        super(Tests, self).setUp()
        self.input_count = 0

    def newWidget(self):
        self.test_input_file = "../../common/transient.i"
        widget = CheckInputWidget()
        widget.needInputFile.connect(self.needInputFile)
        widget.show()
        widget.path = Testing.find_moose_test_exe()
        return widget

    def needInputFile(self, input_file):
        self.input_count += 1
        self.input_file = input_file
        data = None
        with open(self.test_input_file, "r") as f:
            data = f.read()

        with open(input_file, "w") as f:
            f.write(data)

    def testBad(self):
        w = self.newWidget()
        w.cleanup()
        w.cleanup()
        w.check("/No_exist")

    def testCheckInputWidget(self):
        w = self.newWidget()
        self.assertEqual(self.input_count, 0)
        self.assertTrue(w.isVisible())
        QTest.mouseClick(w.check_button, Qt.LeftButton)
        self.assertEqual(self.input_count, 1)
        self.assertIn("Syntax OK", str(w.output.toPlainText()))
        QTest.mouseClick(w.hide_button, Qt.LeftButton)
        self.assertFalse(w.isVisible())


if __name__ == '__main__':
    Testing.run_tests()
