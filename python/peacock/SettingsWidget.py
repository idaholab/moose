from PyQt5.QtWidgets import QWidget, QTabWidget
from PyQt5.QtCore import QSettings
from peacock.utils import WidgetUtils
from peacock.base import MooseWidget

class SettingsWidget(QWidget, MooseWidget):
    """
    A very simple widget to modify settings.
    This widget actually doesn't do anything, it just holds
    various tabs.
    Other widgets are responsible for creating their own settings
    widget and adding it here.
    """
    def __init__(self, **kwds):
        """
        Constructor.
        """
        super(SettingsWidget, self).__init__(**kwds)

        self.top_layout = WidgetUtils.addLayout(vertical=True)
        self.setLayout(self.top_layout)
        self.tabs = QTabWidget(parent=self)
        self.top_layout.addWidget(self.tabs)
        self.button_layout = WidgetUtils.addLayout()
        self.top_layout.addLayout(self.button_layout)
        self.save_button = WidgetUtils.addButton(self.button_layout, self, "&Save", self._save)
        self.cancel_button = WidgetUtils.addButton(self.button_layout, self, "&Cancel", self._cancel)
        self.setup()

    def addTab(self, name, widget, index=-1):
        """
        Add a new tab for settings.
        Input:
            name: name of the tab
            widget: widget of settings
            index: index of where to put the tab. Default is at the beginning.
        """
        self.removeTab(name)
        self.tabs.insertTab(index, widget, name)

    def removeTab(self, name):
        """
        Remove a tab with a given name.
        Input:
            name: name of the tab. If it doesn't exist, nothing happens.
        """
        for i in range(self.tabs.count()):
            if self.tabs.tabText(i) == name:
                self.tabs.removeTab(i)
                break

    def load(self):
        """
        Loads all the settings from the different widgets.
        """
        settings = QSettings()
        for i in range(self.tabs.count()):
            w = self.tabs.widget(i)
            w.load(settings)

    def _save(self):
        """
        Saves all the settings from the different widgets.
        """
        settings = QSettings()
        for i in range(self.tabs.count()):
            w = self.tabs.widget(i)
            w.save(settings)
        settings.sync()
        self.close()

    def _cancel(self):
        """
        They didn't want to save.
        """
        self.close()
