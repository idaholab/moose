#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from peacock.Execute.TerminalTextEdit import TerminalTextEdit
from PyQt5 import QtTest
from PyQt5.QtWidgets import QMenu, QFileDialog, QApplication
from mock import patch
import tempfile
from peacock.utils import Testing

class MockEvent(object):
    def globalPos(self):
        return None

class Tests(Testing.PeacockTester):
    qapp = QApplication([])

    def testClear(self):
        t = TerminalTextEdit()
        QtTest.QTest.keyClicks(t, "Test input")
        # TerminalTextEdit is read only
        self.assertEqual(str(t.toPlainText()), "")
        t.setPlainText("Test input")
        self.assertEqual(str(t.toPlainText()), "Test input")
        t.clear()
        self.assertEqual(str(t.toPlainText()), "")

    @patch.object(QFileDialog, "getSaveFileName")
    def testSave(self, mock_file):
        t = TerminalTextEdit()
        mock_file.return_value = "/no_exist/no_exist", None
        t.setPlainText("Test input")
        t.save()
        with tempfile.NamedTemporaryFile() as f:
            mock_file.return_value = f.name, None
            t.save()
            with open(f.name, "r") as f:
                data = f.read()
                self.assertEqual(data, "Test input")

    def menuExec(self, point):
        if self.do_save_action:
            return "Save"
        else:
            return "Clear"

    @patch.object(QMenu, 'exec_')
    @patch.object(QMenu, 'addAction')
    @patch.object(TerminalTextEdit, "save")
    def testContextMenu(self, mock_save, mock_add, mock_exec):
        t = TerminalTextEdit()
        self.action_count = 0
        self.do_save_action = True
        mock_add.side_effect = ["Save", "Clear"]
        mock_exec.side_effect = self.menuExec
        t.setPlainText("Test input")

        self.assertEqual(mock_save.call_count, 0)
        t.contextMenuEvent(MockEvent())
        self.assertEqual(t.toPlainText(), "Test input")
        self.assertEqual(mock_save.call_count, 1)

        self.do_save_action = False
        mock_add.side_effect = ["Save", "Clear"]
        t.contextMenuEvent(MockEvent())
        self.assertEqual(t.toPlainText(), "")
        self.assertEqual(mock_save.call_count, 1)

if __name__ == '__main__':
    Testing.run_tests()
