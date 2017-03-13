from PyQt5 import QtCore, QtWidgets
from MooseWidget import MooseWidget
from peacock.utils import WidgetUtils

class ViewerCornerWidget(QtWidgets.QWidget, MooseWidget):
    """
    Widget for cloning/closing postprocessor tabs.
    """

    #: pyqtSignal: Emitted when the close button is pressed
    close = QtCore.pyqtSignal()

    #: pyqtSingal: Emitted when the clone button is pressed
    clone = QtCore.pyqtSignal()

    def __init__(self):
        super(ViewerCornerWidget, self).__init__()

        self._icon_size = QtCore.QSize(24, 24)

        self.MainLayout = QtWidgets.QHBoxLayout()
        self.MainLayout.setContentsMargins(0, 0, 0, 0)
        self.MainLayout.setSpacing(10)
        self.setLayout(self.MainLayout)

        self.CloseButton = QtWidgets.QPushButton(WidgetUtils.createIcon('close.ico'), 'Close')
        self.CloneButton = QtWidgets.QPushButton(WidgetUtils.createIcon('copy.ico'), 'Clone')

        #self.MainLayout.addStretch()
        self.MainLayout.addWidget(self.CloneButton)
        self.MainLayout.addWidget(self.CloseButton)

        self.setup()
        self.adjustSize()

    def _setupCloseButton(self, qobject):
        """
        Setup method for close button.
        """
        qobject.setEnabled(False)
        qobject.clicked.connect(self._callbackCloseButton)

    def _callbackCloseButton(self):
        """
        Callback for close button.
        """
        self.close.emit()

    def _setupCloneButton(self, qobject):
        """
        Setup method for clone button.
        """
        qobject.clicked.connect(self._callbackCloneButton)

    def _callbackCloneButton(self):
        """
        Callback for clone button.
        """
        self.clone.emit()
