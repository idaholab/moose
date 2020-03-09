#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from peacock.base.Plugin import Plugin
from .TerminalTextEdit import TerminalTextEdit

class ConsoleOutputViewerPlugin(TerminalTextEdit, Plugin):
    """
    Just adds plugin functionality to the TerminalTextEdit
    """
    def __init__(self, **kargs):
        super(ConsoleOutputViewerPlugin, self).__init__(**kargs)
        self.do_scroll = True
        self.vert_bar = self.verticalScrollBar()
        self.vert_bar.valueChanged.connect(self.viewPositionChanged)
        self._preferences.addInt("execute/terminalFontSize",
                "Execute output font size",
                12,
                1,
                30,
                "Set the output font size for the terminal",
                )
        self.onPreferencesSaved()

    def onOutputAdded(self, text):
        """
        Adds output and scrolls if we are at the bottom.
        Input:
            text[str]: Text to be added
        """
        self.append(text)
        if self.do_scroll:
            self.vert_bar.setValue(self.vert_bar.maximum())

    def onPreferencesSaved(self):
        self.setFontSize(self._preferences.value("execute/terminalFontSize"))

    def onClearLog(self):
        """
        Clear the log
        """
        self.clear()

    def onSaveLog(self):
        """
        Save the log
        """
        self.save()

    def viewPositionChanged(self, val):
        """
        The view changed.
        This can be due to adding additional text
        or if the user scroll around the text window.
        If we are at the bottom then we turn on auto scrolling.
        Input:
            val[int]: The current position of the vertical bar slider
        """
        self.do_scroll = val == self.vert_bar.maximum()

if __name__ == "__main__":
    from PyQt5.QtWidgets import QApplication
    import sys
    qapp = QApplication(sys.argv)
    w = ConsoleOutputViewerPlugin()
    w.append('<span style="color:red;">foo</span>')
    w.show()
    w.setEnabled(True)
    sys.exit(qapp.exec_())
