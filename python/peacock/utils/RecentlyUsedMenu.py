#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from PyQt5.QtCore import pyqtSignal, pyqtSlot, QSettings, QObject
from PyQt5.QtWidgets import QAction
from peacock.base.MooseWidget import MooseWidget

class RecentlyUsedMenu(QObject, MooseWidget):
    """
    Keeps a list of recently used items, in order of use
    """
    selected = pyqtSignal(str)
    def __init__(self, menu, recent_key, max_recent_key, max_recent_default, *args, **kwds):
        super(RecentlyUsedMenu, self).__init__(*args, **kwds)

        self._recent_key = recent_key
        self._max_recent_key = max_recent_key
        self._max_recent_default = max_recent_default
        self._menu = menu
        settings = QSettings()
        self._values = self._getSettingsValue(settings, self._recent_key, [])
        self._max_val = settings.value(self._max_recent_key)
        if self._max_val is None:
            settings.setValue(self._max_recent_key, self._max_recent_default)
            self._max_val = self._max_recent_default
        self.updateRecentlyOpened()

        self.setup()

    def _getSettingsValue(self, settings, key, default):
        val = settings.value(key)
        if val == None:
            return default
        return [str(x) for x in val]

    def _indexOf(self, l, val):
        try:
            idx = l.index(val)
            return idx
        except ValueError:
            return -1

    @pyqtSlot()
    def _fileSelected(self):
        action = self.sender()
        if action:
            name = str(action.text())
            self.update(name)
            self.selected.emit(name)

    def update(self, name):
        if not name:
            return
        self.updateRecentlyUsed(name)
        self.updateRecentlyOpened()

    def entryCount(self):
        return len(self._values)

    def clearValues(self):
        settings = QSettings()
        settings.setValue(self._recent_key, [])
        self._values = []
        self.updateRecentlyOpened()

    def removeEntry(self, value):
        settings = QSettings()
        if self._values:
            idx = self._indexOf(self._values, value)
            if idx >= 0:
                self._values = self._values[:idx] + self._values[(idx+1):]
                settings.setValue(self._recent_key, self._values)
                for action in self._menu.actions():
                    if action.text() == value:
                        self._menu.removeAction(action)
        self.updateRecentlyOpened()

    def updateRecentlyUsed(self, value):
        settings = QSettings()
        if self._values:
            idx = self._indexOf(self._values, value)
            if idx >= 0:
                self._values = self._values[:idx] + self._values[(idx+1):]
                self._values.insert(0, value)
            else:
                self._values.insert(0, value)
                max_files = settings.value(self._max_recent_key, type=int)
                if not max_files:
                    max_files = self._max_val
                    settings.setValue(self._max_recent_key, max_files)
                if len(self._values) >= max_files:
                    self._values.pop()
        else:
            self._values = [value]
        settings.setValue(self._recent_key, self._values)
        return self._values

    def updateRecentlyOpened(self):
        self._menu.clear()
        if not self._values:
            self._menu.setEnabled(False)
            return
        self._menu.setEnabled(True)
        for i, value in enumerate(self._values):
            if i < int(self._max_val):
                action = QAction(value, self)
                action.triggered.connect(self._fileSelected)
                self._menu.addAction(action)

    def setEnabled(self, val):
        if val and self._values:
            self._menu.setEnabled(val)
        else:
            self._menu.setEnabled(False)
