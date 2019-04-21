#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import re
import sys
from PyQt5 import QtCore, QtWidgets
import chigger
from .ExodusPlugin import ExodusPlugin

class ContourPlugin(QtWidgets.QGroupBox, ExodusPlugin):
    """
    Widget for enabling and controlling contours.
    """

    #: Emitted when the window needs updating
    windowRequiresUpdate = QtCore.pyqtSignal()

    #: Emitted when the contour buttons is clicked
    contourClicked = QtCore.pyqtSignal(bool)

    #: pyqtSignal: Emitted when the chigger objects options are changed
    resultOptionsChanged = QtCore.pyqtSignal(dict)

    #: pyqtSignal: Emitted when adding a filter
    addFilter = QtCore.pyqtSignal(chigger.filters.ChiggerFilterBase)

    #: pyqtSignal: Emitted when removing a filter
    removeFilter = QtCore.pyqtSignal(chigger.filters.ChiggerFilterBase)

    def __init__(self):
        super(ContourPlugin, self).__init__()

        self.setTitle('Contours')
        self.setCheckable(True)
        self.setChecked(False)
        self.MainLayout = QtWidgets.QHBoxLayout(self)
        #self.MainLayout.setSpacing(0)

        self.ContourCountLabel = QtWidgets.QLabel("Count:")
        self.ContourCount = QtWidgets.QSpinBox()
        self.ContourLevelsLabel = QtWidgets.QLabel("Levels:")
        self.ContourLevels = QtWidgets.QLineEdit()

        self.MainLayout.addWidget(self.ContourCountLabel)
        self.MainLayout.addWidget(self.ContourCount)
        self.MainLayout.addSpacing(10)
        self.MainLayout.addWidget(self.ContourLevelsLabel)
        self.MainLayout.addWidget(self.ContourLevels)

        self._contour = chigger.filters.ContourFilter()
        self.clicked.connect(self._callbackClicked)

        self.setup()
        self.store(key='default')
        self._varinfo = None

    def onSetupResult(self, result):
        """
        Store variable information when the reader is created.
        """
        self._varinfo = result[0].getExodusReader().getVariableInformation([chigger.exodus.ExodusReader.NODAL])

    def onWindowReset(self):
        """
        Remove variable information when the window is reset.
        """
        self._varinfo = None

    def _loadPlugin(self):
        """
        Helper for loading plugin state.
        """
        self.load()
        if not self.hasState():
            self.setChecked(False)

    def updateOptions(self):
        """
        Called when contours are toggled or the count or levels are changed.
        """

        # Return if the variable information is not set
        if self._varinfo is None:
            return

        # Enable if the variable is valid for contours
        if self._variable in self._varinfo:
            self.setEnabled(True)
        else:
            self.setEnabled(False)

        checked = self.isChecked()
        self.ContourCount.setEnabled(checked)
        self.ContourLevels.setEnabled(checked)

        # Apply filter settings
        if checked:
            # If visible setup the contour interval/count
            options = dict()
            text = self.ContourLevels.text()
            if len(text) > 0:
                try:
                    options['levels'] = [float(item) for item in re.split('[;,\s]', str(text))]
                    self.ContourCount.setEnabled(False)
                except:
                    self.ContourCount.setEnabled(False)
                    self.ContourLevels.setStyleSheet('color:#ff0000')
                    options['levels'] = None
                    options['count'] = int(self.ContourCount.value())
            else:
                self.ContourCount.setEnabled(True)
                options['levels'] = None
                options['count'] = int(self.ContourCount.value())

            self._contour.setOptions(**options)
            self.addFilter.emit(self._contour)

        else:
            self.removeFilter.emit(self._contour)

    def repr(self):
        """
        Return python scripting content.
        """
        output = dict()
        if self.isChecked():
            options, sub_options = self._contour.options().toScriptString()
            output['filters'] = ['contour = chigger.filters.ContourFilter()']
            output['filters'] += ['contour.setOptions({})'.format(', '.join(options))]
            for key, value in sub_options.iteritems():
                output['filters'] += ['contour.setOptions({}, {})'.format(repr(key), ', '.join(value))]

        return output

    def _callbackClicked(self, status):
        """
        Called when groupbox level checkbox is selected.
        """
        self.store()
        self.updateOptions()
        self.windowRequiresUpdate.emit()

    def _setupContourCount(self, qobject):
        """
        Setup method for the contour counter spinbox.
        """
        qobject.setValue(10)
        qobject.setMinimum(3)
        qobject.setEnabled(True)
        qobject.valueChanged.connect(self._callbackContourCount)

    def _callbackContourCount(self):
        """
        Call with contour count spin box is changed.
        """
        self.store(self.ContourCount)
        self.updateOptions()
        self.windowRequiresUpdate.emit()

    def _setupContourLevels(self, qobject):
        """
        Setup method for the contour level edit.
        """
        qobject.setEnabled(True)
        qobject.editingFinished.connect(self._callbackContourLevels)
        qobject.textEdited.connect(lambda: qobject.setStyleSheet('color:#000000'))

    def _callbackContourLevels(self):
        """
        Called when contour levels are changed.
        """
        self.store(self.ContourLevels)
        self.updateOptions()
        self.windowRequiresUpdate.emit()

def main(size=None):
    """
    Run the ContourPlugin all by its lonesome.
    """
    from peacock.ExodusViewer.ExodusPluginManager import ExodusPluginManager
    from peacock.ExodusViewer.plugins.VTKWindowPlugin import VTKWindowPlugin
    from peacock.ExodusViewer.plugins.FilePlugin import FilePlugin
    from peacock.ExodusViewer.plugins.BlockPlugin import BlockPlugin
    widget = ExodusPluginManager(plugins=[lambda: VTKWindowPlugin(size=size), FilePlugin, ContourPlugin, BlockPlugin])
    widget.show()

    return widget, widget.VTKWindowPlugin

if __name__ == '__main__':
    from peacock.utils import Testing
    app = QtWidgets.QApplication(sys.argv)
    filenames = Testing.get_chigger_input_list('mug_blocks_out.e')
    widget, window = main(size=[600,600])
    widget.FilePlugin.onSetFilenames(filenames)
    sys.exit(app.exec_())
