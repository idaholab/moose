#!/usr/bin/env python
from PyQt5 import QtWidgets
from peacock import PeacockApp
from peacock.utils import Testing
import os

class TestExodusState(Testing.PeacockImageTestCase):
    """
    Test for ExodusViewer state when executable is re-run.
    """
    qapp = QtWidgets.QApplication([])

    def setUp(self):
        """
        Creates the peacock application.
        """

        args = ["-size", "1024", "768", "-i", "../../common/transient_big.i", "-e", Testing.find_moose_test_exe()]
        working_dir = os.getcwd()
        self._app = PeacockApp.PeacockApp(args, self.qapp)
        os.chdir(working_dir)
        self._window = self._app.main_widget.tab_plugin.ExodusViewer.currentWidget().VTKWindowPlugin
        Testing.set_window_size(self._window)
        Testing.remove_file('peacock_run_exe_tmp_out.e')

    def selectTab(self, tab):
        """
        Helper function for toggling tabs.
        """
        self._app.main_widget.tab_plugin.setCurrentWidget(tab)
        self._app.main_widget.tab_plugin.currentChanged.emit(self._app.main_widget.tab_plugin.currentIndex())
        Testing.process_events(self.qapp, t=1)

    def execute(self):
        """
        Helper for running executable.
        """
        execute = self._app.main_widget.tab_plugin.ExecuteTabPlugin
        execute.ExecuteRunnerPlugin.runClicked()
        Testing.process_events(self.qapp, t=5)

    def testAutoRangeUpdate(self):
        """
        Test that the auto range is updating correctly.
        """

        # Execute the test
        exodus = self._app.main_widget.tab_plugin.ExodusViewer
        self.selectTab(exodus)
        self.execute()

        self.assertAlmostEqual(0.004492983, float(exodus.currentWidget().VariablePlugin.RangeMinimum.text()))
        self.assertAlmostEqual(0.128, float(exodus.currentWidget().VariablePlugin.RangeMaximum.text()))


if __name__ == '__main__':
    Testing.run_tests()
