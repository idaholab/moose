#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from peacock.BasePeacockMainWindow import BasePeacockMainWindow
from peacock.utils import Testing
from peacock.Input.InputFileEditorWithMesh import InputFileEditorWithMesh
from peacock.Execute.ExecuteTabPlugin import ExecuteTabPlugin
import argparse
from mooseutils import message
from PyQt5 import QtWidgets

class Tests(Testing.PeacockTester):
    qapp = QtWidgets.QApplication([])

    def setUp(self):
        super(Tests, self).setUp()
        self.input_count = 0
        self.input_path = None
        message.MOOSE_DEBUG_MODE = False

    def newWidget(self, args=[]):
        plugins = [ExecuteTabPlugin, InputFileEditorWithMesh]
        parser = argparse.ArgumentParser()
        BasePeacockMainWindow.commandLineArgs(parser, plugins)
        w = BasePeacockMainWindow(plugins=plugins)
        w.show()
        w.initialize(parser.parse_args(args))
        return w

    def testOptions(self):
        # Just get some coverage
        self.newWidget(args=["-d"])
        self.assertEqual(message.MOOSE_DEBUG_MODE, True)

        self.newWidget(args=["--clear-recently-used"])

    def testWindows(self):
        w = self.newWidget()
        self.assertEqual(w.log.isVisible(), False)
        w._showLog()
        self.assertEqual(w.log.isVisible(), True)
        message.mooseMessage("Foobar")
        self.assertIn("Foobar", w.log.log.toPlainText())
        w._showLog()
        self.assertEqual(w.log.isVisible(), False)

        self.assertEqual(w.settings, None)
        w._showPreferences()
        self.assertNotEqual(w.settings, None)
        self.assertEqual(w.settings.isVisible(), True)

    def testSizes(self):
        w = self.newWidget(args=["-full"])
        self.assertEqual(w.isFullScreen(), True)
        self.assertEqual(w.isMaximized(), False)

        w = self.newWidget(args=["-max"])
        self.assertEqual(w.isFullScreen(), False)
        self.assertEqual(w.isMaximized(), True)

        w = self.newWidget(args=["-size", "2048", "2048"])
        self.assertEqual(w.isFullScreen(), False)
        self.assertEqual(w.isMaximized(), False)
        self.assertEqual(w.size().width(), 2048)
        self.assertEqual(w.size().height(), 2048)

    def testPrefs(self):
        w = self.newWidget()
        w._showPreferences()
        self.assertEqual(w.settings.tabs.count(), 2) # Only prefs for Execute and Input tabs
        self.assertEqual(w.settings.isVisible(), True)
        exe_prefs = w.settings.widget(w.tab_plugin.ExecuteTabPlugin.tabName())
        mpiEnabled = exe_prefs.widget("execute/mpiEnabled")
        mpiEnabled.setValue(True)

        w.settings.save_button.click()
        self.assertEqual(w.settings.isVisible(), False)
        self.assertEqual(mpiEnabled.getValue(), True)

        w._showPreferences()
        self.assertEqual(w.settings.isVisible(), True)
        mpiEnabled.setValue(False)
        w.settings.cancel_button.click()
        self.assertEqual(w.settings.isVisible(), False)
        self.assertEqual(mpiEnabled.getValue(), True)

        w._showPreferences()
        self.assertEqual(mpiEnabled.getValue(), True)
        mpiEnabled.setValue(False)
        w.settings.save_button.click()
        self.assertEqual(mpiEnabled.getValue(), False)

if __name__ == '__main__':
    Testing.run_tests()
