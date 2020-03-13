#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from PyQt5 import QtCore, QtWidgets
from peacock.utils import Testing

class TestExodusState(Testing.PeacockAppImageTestCase):
    """
    Test for ExodusViewer state when executable is re-run.
    """
    qapp = QtWidgets.QApplication([])

    def testState(self):
        """
        Tests that re-executing doesn't change the state of the exodus viewer.
        """

        # The tabs to switch between
        exodus = self._app.main_widget.tab_plugin.ExodusViewer
        execute = self._app.main_widget.tab_plugin.ExecuteTabPlugin

        # Run and check that basic results show up
        self.execute()
        self.selectTab(exodus)
        Testing.process_events(1)
        self.assertImage("testDefault.png")

        # Select the mesh
        mesh_plugin = exodus.currentWidget().MeshPlugin
        mesh_plugin.ViewMeshToggle.setCheckState(QtCore.Qt.Checked)
        mesh_plugin.ViewMeshToggle.clicked.emit(True)
        self.assertImage("testMeshOn.png", allowed=0.98)

        # Re-run and check results again
        self.selectTab(execute)
        self.execute()
        self.selectTab(exodus)
        self.assertImage("testMeshOn.png", allowed=0.98)

    def testTabChange(self):
        """
        Tests that changing tabs chan
        """

        # The tabs to switch between
        exodus = self._app.main_widget.tab_plugin.ExodusViewer
        execute = self._app.main_widget.tab_plugin.ExecuteTabPlugin

        # Execute tab active, but nothing run yet (both timers should be inactive)
        self.assertFalse(self._window._timers['initialize'].isActive())
        self.assertFalse(self._window._timers['update'].isActive())

        # Switch to Exodus tabs (initialize timer should be running)
        self.selectTab(exodus)
        self.assertTrue(self._window._timers['initialize'].isActive())
        self.assertFalse(self._window._timers['update'].isActive())

        # Execute (update timer should be running, initialize should be off)
        self.execute()
        self.assertFalse(self._window._timers['initialize'].isActive())
        self.assertTrue(self._window._timers['update'].isActive())

        # Switch to Execute tab (both timers should be disabled)
        self.selectTab(execute)
        self.assertFalse(self._window._timers['initialize'].isActive())
        self.assertFalse(self._window._timers['update'].isActive())

    def testColorbarState(self):
        """
        Test that re-running the simulation maintains colorbar state.
        """
        # The tabs to switch between
        exodus = self._app.main_widget.tab_plugin.ExodusViewer
        execute = self._app.main_widget.tab_plugin.ExecuteTabPlugin
        cbar_plugin = exodus.currentWidget().ColorbarPlugin

        # Run and check that basic results show up
        self.execute()
        self.selectTab(exodus)
        Testing.process_events(1)

        # Disable colorbar
        cbar_plugin.ColorBarToggle.setCheckState(QtCore.Qt.Unchecked)
        cbar_plugin.ColorBarToggle.clicked.emit(True)
        self.assertImage("testColorbarOff.png", allowed=0.98)

        # Re-run and check results again
        self.selectTab(execute)
        self.execute()
        self.selectTab(exodus)
        Testing.process_events(1)
        self.assertImage("testColorbarOff.png", allowed=0.98)


if __name__ == '__main__':
    Testing.run_tests()
