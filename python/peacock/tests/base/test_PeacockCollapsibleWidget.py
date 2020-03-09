#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import sys
import unittest
from PyQt5 import QtWidgets
import peacock

class TestPeacockCollapsibleWidget(unittest.TestCase):
    """
    Test collapsible regions.
    """
    qapp = QtWidgets.QApplication(sys.argv)
    def testCollapseDefault(self):
        collapse = peacock.base.PeacockCollapsibleWidget(title='The Title')
        main = collapse.collapsibleLayout()

        widget = QtWidgets.QWidget()
        main.addWidget(widget)

        self.assertFalse(collapse.isCollapsed())
        collapse._callbackHideButton()
        self.assertTrue(collapse.isCollapsed())
        collapse._callbackHideButton()
        self.assertFalse(collapse.isCollapsed())

        self.assertEqual(collapse._title_widget.text(), 'The Title')

    def testCollapseStartCollapse(self):
        collapse = peacock.base.PeacockCollapsibleWidget(collapsed=True, title='The Title')
        main = collapse.collapsibleLayout()

        widget = QtWidgets.QWidget()
        main.addWidget(widget)

        self.assertTrue(collapse.isCollapsed())
        collapse._callbackHideButton()
        self.assertFalse(collapse.isCollapsed())
        collapse._callbackHideButton()
        self.assertTrue(collapse.isCollapsed())

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
