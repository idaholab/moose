#!/usr/bin/env python
from PyQt5 import QtCore
from peacock.utils import Testing

class TestExodusState(Testing.PeacockAppImageTestCase):
    """
    Test for ExodusViewer state when executable is re-run.
    """
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

if __name__ == '__main__':
    Testing.run_tests()
