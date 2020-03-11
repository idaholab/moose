#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import unittest
from PyQt5 import QtCore, QtWidgets
from peacock import PeacockApp
from peacock.utils import Testing
import os

class TestPostprocessorState(Testing.PeacockImageTestCase):
    """
    Test for ExodusViewer state when executable is re-run.
    """
    qapp = QtWidgets.QApplication([])

    def setUp(self):
        """
        Creates the peacock application.
        """
        Testing.setupTestCache(self.__class__)
        args = ["-size", "1024", "768", "-i", "../../common/time_data.i", "-e", Testing.find_moose_test_exe(), "-w", os.getcwd()]
        self._app = PeacockApp.PeacockApp(args, self.qapp)
        self._window = self._app.main_widget.tab_plugin.VectorPostprocessorViewer.currentWidget().FigurePlugin
        Testing.set_window_size(self._window)
        Testing.remove_file('peacock_run_exe_tmp_out.e')

    def selectTab(self, tab):
        """
        Helper function for toggling tabs.
        """
        self._app.main_widget.tab_plugin.setCurrentWidget(tab)
        self._app.main_widget.tab_plugin.currentChanged.emit(self._app.main_widget.tab_plugin.currentIndex())
        Testing.process_events(t=1)

    def execute(self):
        """
        Helper for running executable.
        """
        execute = self._app.main_widget.tab_plugin.ExecuteTabPlugin
        execute.ExecuteRunnerPlugin.runClicked()
        execute.ExecuteRunnerPlugin.runner.process.waitForFinished()
        Testing.process_events(t=1)

    @unittest.skip("Broken by #12702")
    def testState(self):
        """
        Tests that re-executing doesn't change the state of the exodus viewer.
        """

        # The tabs to switch between
        vpp = self._app.main_widget.tab_plugin.VectorPostprocessorViewer
        execute = self._app.main_widget.tab_plugin.ExecuteTabPlugin
        execute.ExecuteOptionsPlugin.csv_checkbox.setCheckState(QtCore.Qt.Unchecked)

        # Run and check that basic results show up
        self.execute()

        # Plot something on VPP tab
        self.selectTab(vpp)
        toggle = vpp.currentWidget().PostprocessorSelectPlugin._groups[0]._toggles['u']
        toggle.CheckBox.setCheckState(QtCore.Qt.Checked)
        toggle.CheckBox.clicked.emit(True)
        self.assertImage("testDefault.png")

        # Re-run and check results again
        self.selectTab(execute)
        self.execute()
        self.selectTab(vpp)
        self.assertImage("testDefault.png")

    @unittest.skip("Broken by #12702")
    def testTabChange(self):
        """
        Tests that changing tabs disables data update
        """

        # The tabs to switch between
        vpp = self._app.main_widget.tab_plugin.VectorPostprocessorViewer
        execute = self._app.main_widget.tab_plugin.ExecuteTabPlugin
        execute.ExecuteOptionsPlugin.csv_checkbox.setCheckState(QtCore.Qt.Unchecked)
        self.execute()

        # Execute tab active, VPP timer off
        def get_active():
            return any([group._data._timer.isActive() for group in vpp.currentWidget().PostprocessorSelectPlugin._groups])
        self.assertFalse(get_active())

        # VPP tab active, VPP timer on
        self.selectTab(vpp)
        self.assertTrue(get_active())

if __name__ == '__main__':
    Testing.run_tests()
