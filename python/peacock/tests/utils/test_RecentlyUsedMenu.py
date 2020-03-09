#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from peacock.utils.RecentlyUsedMenu import RecentlyUsedMenu
from peacock.utils import Testing
from PyQt5 import QtWidgets

class Tests(Testing.PeacockTester):
    qapp = QtWidgets.QApplication([])

    def setUp(self):
        super(Tests, self).setUp()
        self.main_win = QtWidgets.QMainWindow()
        self.menubar = self.main_win.menuBar()
        self.test_menu = self.menubar.addMenu("Test recently used")
        self.ru_menu = RecentlyUsedMenu(self.test_menu, "test/recentlyUsed", "test/maxRecentlyUsed", 20)
        self.ru_menu.selected.connect(self.selected)
        self.name_selected = None

    def selected(self, name):
        self.name_selected = name

    def test_RecentlyUsedMenu(self):
        self.assertEqual(self.ru_menu._values, [])
        self.assertEqual(self.ru_menu._menu.actions(), [])

        self.ru_menu.update("test_entry")
        self.assertEqual(self.ru_menu._values, ["test_entry"])
        self.assertEqual(len(self.ru_menu._menu.actions()), 1)

        self.ru_menu.removeEntry("test_entry")
        self.assertEqual(self.ru_menu._values, [])
        self.assertEqual(self.ru_menu._menu.actions(), [])

        self.ru_menu.update("test_entry")
        self.assertEqual(self.ru_menu._values, ["test_entry"])
        self.assertEqual(len(self.ru_menu._menu.actions()), 1)
        self.ru_menu._menu.actions()[0].triggered.emit()
        self.assertEqual(self.name_selected, "test_entry")

        self.ru_menu.removeEntry("no_exist")
        self.assertEqual(self.ru_menu._values, ["test_entry"])
        self.assertEqual(len(self.ru_menu._menu.actions()), 1)

if __name__ == '__main__':
    Testing.run_tests()
