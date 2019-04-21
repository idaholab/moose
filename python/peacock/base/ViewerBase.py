#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from PyQt5 import QtWidgets
from .TabPlugin import TabPlugin
from .ViewerCornerWidget import ViewerCornerWidget
import mooseutils

class ViewerBase(QtWidgets.QTabWidget, TabPlugin):
    """
    Base for creating multiple copies of a PluginManager widget.

    @see PostprocessorViewer, ExodusViewer
    """

    def __init__(self, manager=None, plugins=[]):
        super(ViewerBase, self).__init__()

        # Error checking
        if not plugins:
            mooseutils.mooseError("No plugins supplied.")
            return
        if not manager:
            mooseutils.mooseError("The type of manager to create must be supplied.")

        # Member variables and class settings
        self._manager = manager # the manager class to create
        self._plugins = plugins # the plugin classes (not instances) to created
        self._count = 0         # the number of tabs created
        self._data = None       # data to pass to PluginManager::initialize (see 'initialize')

        # Add the corner close/clone buttons
        self.setCornerWidget(ViewerCornerWidget())
        self.cornerWidget().close.connect(self.onClose)
        self.cornerWidget().clone.connect(self.onClone)
        self.currentChanged.connect(self.onCurrentChanged)
        self.setup()
        self.addManager()

    def onClose(self):
        """
        Slot that executs when the close signal is emitted from the CornerWidget object.
        """
        if self.count() > 1:
            self.currentWidget().deleteLater()
            self.removeTab(self.currentIndex())
            if self.count() == 1:
                self.cornerWidget().CloseButton.setEnabled(False)

    def onClone(self):
        """
        Slot that executes when the close signal is emitted from the CornerWidget object.
        """
        self.addManager()
        self.setCurrentIndex(self.currentIndex() + 1)
        self.cornerWidget().CloseButton.setEnabled(True)

    def addManager(self):
        """
        Helper method for adding a tab clone.
        """

        self._count += 1
        if self._count > 1:
            name = "Results ({})".format(self._count)
        else:
            name = "Results"
        tab = self._manager(plugins=self._plugins)
        self.addTab(tab, name)

    def setTabIndex(self, index, signal=None):
        """
        Set the Peacock tab index.
        """
        self._index = index
        for i in range(self.count()):
            self.widget(i).setTabIndex(index, signal)

    def preferencesWidget(self):
        """
        Return the preference widget.
        We only need one widget, so we just use the current tab
        since they should all be the same.
        """
        return self.currentWidget().preferencesWidget()

    def onCurrentChanged(self, index):
        pass
