#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from PyQt5 import QtWidgets
class TextSubWindow(QtWidgets.QTextEdit):
    """
    TextEdit that saves it size when it closes and closes itself if the main widget disappears.
    """
    def __init__(self):
        super(TextSubWindow, self).__init__()
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
