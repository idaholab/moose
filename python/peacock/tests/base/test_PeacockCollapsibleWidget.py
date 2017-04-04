#!/usr/bin/env python
import sys
import unittest
from PyQt5 import QtWidgets
import peacock

class TestPeacockCollapsibleWidget(unittest.TestCase):
    """
    Test collapsible regions.
    """
    def testCollapseDefault(self):
        app = QtWidgets.QApplication(sys.argv)
        self.assertIsNotNone(app)
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
        app = QtWidgets.QApplication(sys.argv)
        self.assertIsNotNone(app)

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
