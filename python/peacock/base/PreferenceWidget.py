#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from PyQt5 import QtWidgets, QtCore

class PreferenceWidget(QtWidgets.QWidget):
    """
    Holds a collection of preferences.
    Signals:
        saved: Emitted when preferences have been saved.
    """
    saved = QtCore.pyqtSignal()

    def __init__(self, plugins):
        super(PreferenceWidget, self).__init__()
        self._widgets = []
        for plugin in plugins:
            self._widgets += plugin.preferenceWidgets()
            self.saved.connect(plugin.onPreferencesSaved)

        if self._widgets:
            layout = QtWidgets.QGridLayout()
            for i, w in enumerate(self._widgets):
                layout.addWidget(w.label(), i, 0)
                layout.addWidget(w.widget(), i, 1)
            self.setLayout(layout)

    def save(self, settings):
        """
        Go through all the preferences and save then to disk
        Input:
            settings[QSettings]: settings to load from
        """
        for w in self._widgets:
            w.save(settings)
        self.saved.emit()

    def load(self, settings):
        """
        Iniitialize each widget from previously saved settings.
        Input:
            settings[QSettings]: settings to load from
        """
        for w in self._widgets:
            w.load(settings)

    def widget(self, key):
        """
        Get a preference widget corresponding to a given key.
        Input:
            key[str]: The key value that is stored in a QSettings
        """
        for w in self._widgets:
            if key == w.key():
                return w
        return None

    def count(self):
        """
        Returns the number of preferences widgets
        """
        return len(self._widgets)
