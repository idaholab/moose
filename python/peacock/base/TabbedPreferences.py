#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from PyQt5 import QtWidgets, QtCore
from peacock.utils import WidgetUtils

class TabbedPreferences(QtWidgets.QWidget):
    """
    For each plugin, store a preference widget in its own tab.
    """
    def __init__(self, plugins):
        super(TabbedPreferences, self).__init__()
        self._widgets = []
        self.layout = QtWidgets.QVBoxLayout()
        self.setLayout(self.layout)
        self.tabs = QtWidgets.QTabWidget(parent=self)
        self.layout.addWidget(self.tabs)
        self.button_layout = QtWidgets.QHBoxLayout()
        self.layout.addLayout(self.button_layout)
        self.save_button = WidgetUtils.addButton(self.button_layout, self, "&Save", self.save)
        self.cancel_button = WidgetUtils.addButton(self.button_layout, self, "&Cancel", self.cancel)

        for plugin in plugins:
            w = plugin.preferencesWidget()
            if w.count() > 0:
                self._widgets.append(w)
                self.tabs.addTab(w, plugin.tabName())

    def save(self):
        """
        Save the preferences to disk
        """
        settings = QtCore.QSettings()
        for w in self._widgets:
            w.save(settings)
        settings.sync()
        self.close()

    def load(self):
        """
        Load preferences from disk.
        """
        settings = QtCore.QSettings()
        for w in self._widgets:
            w.load(settings)

    def widget(self, tab_name):
        """
        Gets the PreferenceWidget based on tab name.
        This is primarily intended for use while testing.
        """
        for i in range(self.tabs.count()):
            if self.tabs.tabText(i) == tab_name:
                return self.tabs.widget(i)
        return None

    def cancel(self):
        """
        Cancel the changes and close the window
        """
        self.load() # we want to leave the widgets in a good state
        self.close()
