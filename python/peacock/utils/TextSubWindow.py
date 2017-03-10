from PyQt5 import QtCore, QtWidgets
class TextSubWindow(QtWidgets.QTextEdit):
    """
    TextEdit that saves it size when it closes and closes itself if the main widget disappears.
    """
    def __init__(self):
        super(TextSubWindow, self).__init__()
        self.setWindowFlags(QtCore.Qt.SubWindow)
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
        super(TextSubWindow, self).closeEvent(*args)
