from PyQt5 import QtWidgets
from TabPlugin import TabPlugin
from ViewerCornerWidget import ViewerCornerWidget
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
        self.setup()
        self.addManager()

    def initialize(self, *args, **kwargs):
        """
        Initialize the widget by creating a tab and supplying data to the PluginManager initialize method.
        """
        super(ViewerBase, self).initialize(*args, **kwargs)
        for i in range(self.count()):
            self.widget(i).initialize(*args, **kwargs)
        self._data = args

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

        widget = self.currentWidget()
        widget.initialize(*self._data)

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
