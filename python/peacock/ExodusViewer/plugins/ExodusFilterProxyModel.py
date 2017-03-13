#!/usr/bin/env python
from PyQt5 import QtWidgets, QtCore
import sys
import os
import re

class ExodusFilterProxyModel(QtCore.QSortFilterProxyModel):
    """
    A filename filter for Exodus *.efiles.
    """
    def filterAcceptsRow(self, row, parent):
        index0 = self.sourceModel().index(row, 0, parent)
        filename = str(self.sourceModel().filePath(index0))

        if os.path.isdir(filename):
            return True

        match = re.search(r'(.*?)\.e(-s[0-9]+)', filename)
        if not match or filename.endswith('.e'):
            return True
        else:
            return False


if __name__ == "__main__":
  qapp = QtWidgets.QApplication(sys.argv)

  fd = QtWidgets.QFileDialog()

  fd.setFileMode(QtWidgets.QFileDialog.ExistingFiles)
  fd.setWindowTitle('Select ExodusII File(s)')
  fd.setDirectory('/Users/slauae/projects/gui/tests/chigger/input')
  fd.setNameFilter('ExodusII Files (*.e)')
  fd.setOption(QtWidgets.QFileDialog.DontUseNativeDialog)

  proxy = ExodusFilterProxyModel(fd)
  fd.setProxyModel(proxy)
  fd.raise_()
  fd.exec_()
