#!/usr/bin/env python
from peacock.Execute.ExecuteSettings import ExecuteSettings
from PyQt5.QtCore import QSettings
from peacock.utils import Testing

class Tests(Testing.PeacockTester):
    def testInputSettings(self):
        settings = QSettings()
        w = ExecuteSettings()
        w.max_exes_spinbox.setValue(2)
        w.max_args_spinbox.setValue(3)
        w.max_working_spinbox.setValue(4)
        w.save(settings)
        w.load(settings)
        self.assertEqual(w.max_exes_spinbox.value(), 2)
        self.assertEqual(w.max_args_spinbox.value(), 3)
        self.assertEqual(w.max_working_spinbox.value(), 4)
        w.max_exes_spinbox.setValue(5)
        w.max_args_spinbox.setValue(6)
        w.max_working_spinbox.setValue(7)
        w.save(settings)
        w.load(settings)
        self.assertEqual(w.max_exes_spinbox.value(), 5)
        self.assertEqual(w.max_args_spinbox.value(), 6)
        self.assertEqual(w.max_working_spinbox.value(), 7)


if __name__ == '__main__':
    Testing.run_tests()
