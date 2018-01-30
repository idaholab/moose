#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from PyQt5 import QtCore, QtWidgets

class PostprocessorTableWidget(QtWidgets.QTabWidget):
    """
    Helper object of having a live table view of postprocessor results.

    see OutputPlugin
    """

    def __init__(self):
        super(PostprocessorTableWidget, self).__init__()
        self.setWindowFlags(QtCore.Qt.SubWindow)
        self._size = None
        self._time = None

    def sizeHint(self, *args):
        """
        Return the saved size.
        """
        if self._size:
            return self._size
        else:
            return super(PostprocessorTableWidget, self).size()

    def closeEvent(self, *args):
        """
        Store the size of the window.
        """
        self._size = self.size()
        super(PostprocessorTableWidget, self).closeEvent(*args)


    def initialize(self, data):
        """
        Called when the data is changed ,this updates the visible data.

        @see OutputPlugin
        """
        self._data = data

    def onTimeChanged(self, time):
        """
        Called by OutputPlugin when the time is altered.
        """

        self._time = time
        self.onDataChanged()

    def onDataChanged(self):
        """
        Called by OutputPlugin when the data has been changed.
        """

        # Current index
        current_index = self.currentIndex()

        # Remove existing tabs
        self.clear()

        # Populate the table
        for data_widget in self._data:

            # Create the table
            idx = self.addTab(QtWidgets.QTableWidget(), data_widget.filename())
            widget = self.widget(idx)

            # Extract all variables and size the table
            variables = data_widget.variables()
            data = data_widget(variables, time=self._time)
            n_rows, n_cols = data.shape
            widget.setRowCount(n_rows)
            widget.setColumnCount(n_cols)

            # Loop through data and set the data
            for c in range(n_cols):
                header = QtWidgets.QTableWidgetItem(str(variables[c]))
                widget.setHorizontalHeaderItem(c, header)
                for r in range(n_rows):
                    item = QtWidgets.QTableWidgetItem(str(data[variables[c]].iloc[r]))
                    widget.setItem(r, c, item)

        # Restore current index
        self.setCurrentIndex(current_index)
