#!/usr/bin/env python
from peacock.utils import Testing
from peacock.PythonConsoleWidget import PythonConsoleWidget
from PyQt5.QtTest import QTest
from PyQt5.QtCore import Qt

class Tests(Testing.PeacockTester):
    def testPythonConsoleWidget(self):
        w = PythonConsoleWidget()
        w.show()
        w.setVariable("foo", "bar")
        self.assertNotIn("bar", w.output.toPlainText())
        QTest.keyClicks(w.input_line, "peacock['foo']")
        QTest.keyClick(w.input_line, Qt.Key_Return)
        self.assertIn("bar", w.output.toPlainText())
        w.output.clear()
        QTest.keyClick(w.input_line, Qt.Key_Up)
        QTest.keyClick(w.input_line, Qt.Key_Return)
        self.assertIn("bar", w.output.toPlainText())
        w.saveHistory()
        del w
        w = PythonConsoleWidget()
        w.show()
        w.setVariable("foo", "bar")
        QTest.keyClick(w.input_line, Qt.Key_Up)
        QTest.keyClick(w.input_line, Qt.Key_Return)
        self.assertIn("bar", w.output.toPlainText())

if __name__ == '__main__':
    Testing.run_tests()
