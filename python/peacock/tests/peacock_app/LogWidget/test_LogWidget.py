#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from peacock.utils import Testing
from peacock.LogWidget import LogWidget
import mooseutils
from PyQt5.QtTest import QTest
from PyQt5.QtCore import Qt
from PyQt5 import QtWidgets

class Tests(Testing.PeacockTester):
    qapp = QtWidgets.QApplication([])

    def testLogWidget(self):
        w = LogWidget()
        w.show()
        mooseutils.mooseMessage("Foo", color="RED")
        self.assertIn("Foo", w.log.toHtml())
        self.assertEqual("Foo\n", w.log.toPlainText())
        QTest.mouseClick(w.clear_button, Qt.LeftButton, delay=100)
        self.assertEqual("", w.log.toPlainText())
        self.assertEqual(w.isVisible(), True)
        QTest.mouseClick(w.hide_button, Qt.LeftButton, delay=100)
        self.assertEqual(w.isVisible(), False)

        mooseutils.mooseMessage("Foo\nbar", color="GREEN")
        self.assertIn("Foo", w.log.toHtml())
        self.assertIn("bar", w.log.toHtml())
        self.assertEqual("Foo\nbar\n\n", w.log.toPlainText())

    def testUnicode(self):
        w = LogWidget()
        w.show()
        mooseutils.mooseMessage("Foo \xe0\xe0 bar", color="RED")
        self.assertIn("Foo", w.log.toHtml())
        self.assertIn("bar", w.log.toHtml())

if __name__ == '__main__':
    Testing.run_tests()
