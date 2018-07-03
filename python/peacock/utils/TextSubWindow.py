#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from PyQt5 import QtWidgets, QtCore
class TextSubWindow(QtWidgets.QTextEdit):
    """
    TextEdit that saves it size when it closes and closes itself if the main widget disappears.
    """
    windowClosed = QtCore.pyqtSignal()

    def __init__(self):
        super(TextSubWindow, self).__init__()
        self.setWindowFlags(QtCore.Qt.SubWindow | QtCore.Qt.CustomizeWindowHint | QtCore.Qt.WindowTitleHint | QtCore.Qt.WindowMinMaxButtonsHint | QtCore.Qt.WindowCloseButtonHint)
        self._size = None

    def sizeHint(self, *args):
        """
        Return the saved size.
        """
        if self._size:
            return self._size
        else:
            return super(TextSubWindow, self).size()

    def closeEvent(self, *args):
        """
        Store the size of the window.
        """
        self._size = self.size()
        self.windowClosed.emit()
        super(TextSubWindow, self).closeEvent(*args)
