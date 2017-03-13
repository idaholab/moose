#!/usr/bin/env python
from PyQt5.QtWidgets import QWidget, QSpinBox
from peacock.utils import WidgetUtils
from peacock.base.MooseWidget import MooseWidget

class InputSettings(QWidget, MooseWidget):
    """
    A simple widget for settings with the input file.
    """
    MAX_RECENT_KEY = "input/maxRecentlyUsed"
    MAX_RECENT_DEFAULT = 20
    RECENTLY_USED_KEY = "input/recentlyUsed"
    def __init__(self, **kwds):
        super(InputSettings, self).__init__(**kwds)

        self.top_layout = WidgetUtils.addLayout(grid=True)
        self.setLayout(self.top_layout)
        label = WidgetUtils.addLabel(None, None, "Max recently used")
        spin = QSpinBox()
        spin.setMinimum(1)
        spin.setMaximum(20)
        self.top_layout.addWidget(label, 0, 0)
        self.top_layout.addWidget(spin, 0, 1)
        self.max_recent_spinbox = spin
        self.setup()

    def save(self, settings):
        """
        Save the settings.
        Input:
            settings: QSettings()
        """
        val = self.max_recent_spinbox.value()
        settings.setValue(self.MAX_RECENT_KEY, val)

    def load(self, settings):
        """
        Load the settings.
        Input:
            settings: QSettings()
        """
        val = settings.value(self.MAX_RECENT_KEY, type=int)
        if val == None:
            val = self.MAX_RECENT_DEFAULT
        self.max_recent_spinbox.setValue(val)

if __name__ == "__main__":
    from PyQt5.QtCore import QSettings
    from peacock.utils import qtutils
    qtutils.setAppInformation("InputSettings")
    settings = QSettings()
    print("Max: %s" % settings.value(InputSettings.MAX_RECENT_KEY, type=int))
    print("Values: %s" % settings.value(InputSettings.RECENTLY_USED_KEY, type=str))
