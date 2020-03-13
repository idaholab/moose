#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from peacock.PeacockMainWindow import PeacockMainWindow
from peacock.utils import Testing
import argparse, os
from PyQt5 import QtWidgets

class Tests(Testing.PeacockTester):
    qapp = QtWidgets.QApplication([])

    def newWidget(self, args=[]):
        parser = argparse.ArgumentParser()
        PeacockMainWindow.commandLineArgs(parser)
        w = PeacockMainWindow()
        w.show()
        w.initialize(parser.parse_args(args))
        return w

    def testConsole(self):
        w = self.newWidget()
        self.assertEqual(w.console.isVisible(), False)
        w._showConsole()
        self.assertEqual(w.console.isVisible(), True)
        w.setPythonVariable("foo", "bar")
        w.tab_plugin.InputFileEditorWithMesh.MeshViewerPlugin._reset()

    def testConnections(self):
        w = self.newWidget(args=[])
        path = Testing.find_moose_test_exe()
        w.tab_plugin.ExecuteTabPlugin.ExecuteOptionsPlugin.setExecutablePath(path)
        self.assertIn(path, w.windowTitle())
        runner = w.tab_plugin.ExecuteTabPlugin.ExecuteRunnerPlugin
        self.assertEqual(runner._total_steps, 0)

        w.tab_plugin.InputFileEditorWithMesh.setInputFile("transient.i")
        w.setTab(w.tab_plugin.ExecuteTabPlugin.tabName())
        self.assertIn("transient.i", w.windowTitle())

        self.assertEqual(runner._total_steps, 8)

        runner.runClicked()
        runner.runner.process.waitForFinished()
        Testing.process_events(t=2)
        self.assertTrue(os.path.exists("out_transient.e"))

        w.tab_plugin.InputFileEditorWithMesh.MeshViewerPlugin._reset()


if __name__ == '__main__':
    Testing.run_tests()
