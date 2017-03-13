#!/usr/bin/env python
from peacock.utils import Testing
from peacock.SettingsWidget import SettingsWidget
from peacock.Execute.ExecuteSettings import ExecuteSettings
from PyQt5.QtTest import QTest
from PyQt5.QtCore import Qt

class Tests(Testing.PeacockTester):
    def testSettingsWidget(self):
        w = SettingsWidget()
        w.show()
        exe_settings = ExecuteSettings()
        w.addTab("Execute", exe_settings)
        self.assertEqual(w.tabs.count(), 1)
        w.load()
        self.assertEqual(w.isVisible(), True)
        QTest.mouseClick(w.save_button, Qt.LeftButton, delay=100)
        self.assertEqual(w.isVisible(), False)

        w.show()
        w.removeTab("Execute")
        self.assertEqual(w.isVisible(), True)
        QTest.mouseClick(w.save_button, Qt.LeftButton, delay=100)
        self.assertEqual(w.tabs.count(), 0)
        self.assertEqual(w.isVisible(), False)

if __name__ == '__main__':
    Testing.run_tests()
