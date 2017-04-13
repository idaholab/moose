#!/usr/bin/env python
from PyQt5.QtWidgets import QWidget, QSpinBox
from peacock.utils import WidgetUtils
from peacock.base import MooseWidget

class ExecuteSettings(QWidget, MooseWidget):
    """
    A simple widget to edit ExecuteTabPlugin settings.
    """
    RECENT_WORKING_KEY = "execute/recentWorkingDirs"
    RECENT_WORKING_MAX_KEY = "execute/maxRecentWorkingDirs"
    RECENT_EXES_KEY = "execute/recentExes"
    RECENT_EXES_MAX_KEY = "execute/maxRecentExes"
    RECENT_ARGS_KEY = "execute/recentArgs"
    RECENT_ARGS_MAX_KEY = "execute/maxRecentArgs"
    MAX_DEFAULT = 20

    def __init__(self, **kwds):
        super(ExecuteSettings, self).__init__(**kwds)

        self.top_layout = WidgetUtils.addLayout(grid=True)
        self.setLayout(self.top_layout)
        tmp, self.max_args_spinbox = self._addOptionToGrid("Max recent working dirs", 0)
        tmp, self.max_exes_spinbox = self._addOptionToGrid("Max recent executables", 1)
        tmp, self.max_working_spinbox = self._addOptionToGrid("Max recent arguments", 2)
        self.setup()

    def _addOptionToGrid(self, name, row):
        label = WidgetUtils.addLabel(None, None, name)
        spin = QSpinBox()
        spin.setMinimum(1)
        spin.setMaximum(20)
        self.top_layout.addWidget(label, row, 0)
        self.top_layout.addWidget(spin, row, 1)
        return label, spin

    def _saveSpin(self, settings, key, spinbox):
        """
        Convienence function to a setting from a spinbox.
        Input:
            settings: QSettings() object
            key: str: key to set in the QSettings
            spinbox: QSpinbox: spinbox with the value
        """
        val = spinbox.value()
        settings.setValue(key, val)

    def save(self, settings):
        """
        Save our setings.
        Input:
            settings: QSettings() object
        """
        self._saveSpin(settings, self.RECENT_ARGS_MAX_KEY, self.max_args_spinbox)
        self._saveSpin(settings, self.RECENT_EXES_MAX_KEY, self.max_exes_spinbox)
        self._saveSpin(settings, self.RECENT_WORKING_MAX_KEY, self.max_working_spinbox)

    def _loadSpin(self, settings, key, spinbox):
        """
        Convienence function to set a spinbox from settings.
        Input:
            settings: QSettings() object
            key: str: key of the setting
            spinbox: QSpinbox: spinbox to set
        """

        val = settings.value(key, type=int)
        if val == None:
            val = self.MAX_DEFAULT
        spinbox.setValue(val)

    def load(self, settings):
        """
        Load settings.
        Input:
            settings: QSettings() object
        """
        self._loadSpin(settings, self.RECENT_ARGS_MAX_KEY, self.max_args_spinbox)
        self._loadSpin(settings, self.RECENT_EXES_MAX_KEY, self.max_exes_spinbox)
        self._loadSpin(settings, self.RECENT_WORKING_MAX_KEY, self.max_working_spinbox)

if __name__ == "__main__":
    from PyQt5.QtCore import QSettings
    from peacock.utils import qtutils
    qtutils.setAppInformation("ExecuteSettings")
    settings = QSettings()
    print("Working Max: %s" % settings.value(ExecuteSettings.RECENT_WORKING_MAX_KEY, type=int))
    print("Working Values: %s" % settings.value(ExecuteSettings.RECENT_WORKING_KEY, type=str))
    print("Exes Max: %s" % settings.value(ExecuteSettings.RECENT_EXES_MAX_KEY, type=int))
    print("Exes Values: %s" % settings.value(ExecuteSettings.RECENT_EXES_KEY, type=str))
    print("Args Max: %s" % settings.value(ExecuteSettings.RECENT_ARGS_MAX_KEY, type=int))
    print("Args Values: %s" % settings.value(ExecuteSettings.RECENT_ARGS_KEY, type=str))
