#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import peacock
from PyQt5 import QtWidgets

class ExodusPlugin(peacock.base.Plugin):
    """
    Plugin class for the Exodus volume rendering portion of Peacock.
    """

    def __init__(self, layout='LeftLayout', settings_key="", **kwargs):
        super(ExodusPlugin, self).__init__(layout=layout, settings_key=settings_key, **kwargs)

        self.setSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.MinimumExpanding)

        # Ubiquitous member variables
        self._filename = None
        self._variable = None
        self._component = -1
        self.setEnabled(False)

    def onPlayStart(self):
        """
        Disables the plugin when playing begins.
        """
        self.setEnabled(False)

    def onPlayStop(self):
        """
        Enables widget when the playing stops.
        """
        self.setEnabled(True)

    def onSetFilename(self, filename):
        """
        Stores the current filename. (see FilePlugin)
        """
        self._filename = str(filename) if filename else None
        self._loadPlugin()
        self.updateOptions()

    def onSetVariable(self, variable):
        """
        Stores the current variable. (see FilePlugin)
        """
        self._variable = str(variable) if variable else None
        self._loadPlugin()
        self.updateOptions()

    def onSetComponent(self, component):
        """
        Stores the current variable component. (see FilePlugin)
        """
        self._component = component if (component is not None) else -1
        self._loadPlugin()
        self.updateOptions()

    def onCurrentChanged(self, index):
        """
        Called by ExodusViewer when the tab is changed.
        """
        pass

    def onSetEnableWidget(self, value):
        """
        Enables/disables the widget after the VTKRenderWindow is created or destroyed.
        """
        self.setEnabled(value)

    def stateKey(self):
        """
        Generates a (mostly) unique key for use in saving state of a widget.
        """
        return (self._filename, self._variable, self._component)

    def setup(self):
        """
        Setup the Exodus widgets with a uniform margins and "flat" style.
        """
        super(ExodusPlugin, self).setup()

        if hasattr(self, 'MainLayout'):
            self.MainLayout.setContentsMargins(5,5,5,5)
            self.MainLayout.setSpacing(5)

        if isinstance(self, QtWidgets.QGroupBox):
            self.setFlat(True)

    def _loadPlugin(self):
        """
        This is called by onSetFilename/Variable/Component, use it to load the plugin state.
        """
        pass

    def updateOptions(self):
        """
        All options for the Reader/Result/Window objects should be set for the plugin by this method.
        """
        pass
