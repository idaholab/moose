#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from PyQt5.QtWidgets import QTextEdit, QMenu, QFileDialog, QSizePolicy
import mooseutils

class TerminalTextEdit(QTextEdit):
    """
    A readonly text edit that replaces terminal codes with appropiate html codes.
    Also uses fixed font.
    """
    def __init__(self, **kwds):
        super(TerminalTextEdit, self).__init__(**kwds)

        self.setStyleSheet("TerminalTextEdit { background: black; color: white; }")
        self.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        self.setReadOnly(True)

    def contextMenuEvent(self, event):
        """
        User requested a context menu.
        Input:
            event: The QEvent()
        """
        menu = QMenu()
        save_action = menu.addAction("Save")
        clear_action = menu.addAction("Clear")
        action = menu.exec_(event.globalPos())

        if action == save_action:
            self.save()
        elif action == clear_action:
            self.clear()

    def save(self):
        """
        Save the contents into a file.
        """
        fname, other = QFileDialog.getSaveFileName(self, "Choose output", "", "Output file (*.log *.txt)")
        if fname:
            try:
                with open(fname, "w") as f:
                    f.write(self.toPlainText())
                mooseutils.mooseMessage("Saved content to %s" % fname)
            except Exception as e:
                mooseutils.mooseError("Failed to save file: %s" % e, dialog=True)

    def clear(self):
        """
        Clear the output
        """
        self.setHtml("")

    def setFontSize(self, size):
        self.setStyleSheet("TerminalTextEdit { background: black; color: white; font: %spx}" % size)

if __name__ == "__main__":
    from PyQt5.QtWidgets import QApplication
    import sys
    qapp = QApplication(sys.argv)
    w = TerminalTextEdit()
    w.append('<span style="color:red;">foo</span>')
    w.show()
    w.setEnabled(True)
    sys.exit(qapp.exec_())
