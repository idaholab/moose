#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from PyQt5 import QtCore, QtWidgets
from peacock.utils import WidgetUtils
from .MooseWidget import MooseWidget

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
