#!/usr/bin/env python
from peacock.Input.InputSettings import InputSettings
from PyQt5.QtCore import QSettings
from peacock.utils import Testing

class Tests(Testing.PeacockTester):
    def testInputSettings(self):
        settings = QSettings()
        w = InputSettings()
        w.max_recent_spinbox.setValue(2)
        w.save(settings)
        w.load(settings)
        self.assertEqual(w.max_recent_spinbox.value(), 2)
        w.max_recent_spinbox.setValue(10)
        w.save(settings)
        w.load(settings)
        self.assertEqual(w.max_recent_spinbox.value(), 10)


if __name__ == '__main__':
    Testing.run_tests()
