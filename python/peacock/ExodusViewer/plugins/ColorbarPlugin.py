#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import chigger
import os
import sys
from PyQt5 import QtCore, QtWidgets, QtGui
import glob
from .ExodusPlugin import ExodusPlugin

class ColorbarPlugin(QtWidgets.QGroupBox, ExodusPlugin):
    """
    Class for controlling variable, range, and colormaps.
    """

    #: pyqtSignal: Emitted when the window needs to be updated
    windowRequiresUpdate = QtCore.pyqtSignal()

    #: pyqtSignal: Emitted when the chigger objects options are changed
    readerOptionsChanged = QtCore.pyqtSignal(dict)
    resultOptionsChanged = QtCore.pyqtSignal(dict)
    colorbarOptionsChanged = QtCore.pyqtSignal(dict)

    def __init__(self):
        super(ColorbarPlugin, self).__init__()

        # Cache for auto limits
        self._auto = [True, True]

        self._preferences.addCombo("exodus/defaultColorMap",
                "Default colormap",
                "default",
                sorted(self._availableColorMaps().keys()),
                "Set the default colormap to use",
                )

        # Min. value selection
        self.RangeMinimumLabel = QtWidgets.QLabel("Min:")
        self.RangeMinimumLabel.setMinimumWidth(30)
        self.RangeMinimumMode = QtWidgets.QCheckBox()
        self.RangeMinimum = QtWidgets.QLineEdit()
        self.RangeMinimumLayout = QtWidgets.QHBoxLayout()
        self.RangeMinimumLayout.addWidget(self.RangeMinimumLabel)
        self.RangeMinimumLayout.addWidget(self.RangeMinimumMode)
        self.RangeMinimumLayout.addWidget(self.RangeMinimum)

        # Max. value selection0
        self.RangeMaximumLabel = QtWidgets.QLabel("Max:")
        self.RangeMaximumLabel.setMinimumWidth(30)
        self.RangeMaximumMode = QtWidgets.QCheckBox()
        self.RangeMaximum = QtWidgets.QLineEdit()
        self.RangeMaximumLayout = QtWidgets.QHBoxLayout()
        self.RangeMaximumLayout.addWidget(self.RangeMaximumLabel)
        self.RangeMaximumLayout.addWidget(self.RangeMaximumMode)
        self.RangeMaximumLayout.addWidget(self.RangeMaximum)

        # Colormap
        self.ColorMapList = QtWidgets.QComboBox()
        self.ColorMapList.setFocusPolicy(QtCore.Qt.StrongFocus)

        # Colorbar toggles
        self.ColorMapReverse = QtWidgets.QCheckBox("Reverse")
        self.ColorBarToggle = QtWidgets.QCheckBox("Colorbar")
        self.ColorBarRangeType = QtWidgets.QCheckBox("Visible Range")
        self.ColorbarToggleLayout = QtWidgets.QHBoxLayout()
        self.ColorbarToggleLayout.addWidget(self.ColorBarToggle)
        self.ColorbarToggleLayout.addWidget(self.ColorMapReverse)
        self.ColorbarToggleLayout.addWidget(self.ColorBarRangeType)
        self.ColorbarToggleLayout.addStretch(1)

        # Add items to the main layout, in proper order
        self.MainLayout = QtWidgets.QVBoxLayout(self)

        self.MainLayout.addLayout(self.RangeMinimumLayout)
        self.MainLayout.addLayout(self.RangeMaximumLayout)
        self.MainLayout.addWidget(self.ColorMapList)
        self.MainLayout.addLayout(self.ColorbarToggleLayout)

        self.MainLayout.setSpacing(0)
        self.RangeMinimumLayout.setSpacing(10)
        self.RangeMaximumLayout.setSpacing(10)
        self.ColorbarToggleLayout.setSpacing(10)

        # The ExodusColorbar object is added by this plugin, so it needs
        # to do a few things with the core chigger objects.
        self._window = None   # RenderWindow (from VTKWindowPlugin)
        self._result = None   # ExodusResult (from VTKWindowPlugin)
        self._colorbar = None # Colorbar (created by this plugin)

        # Call widget setup methods
        self.setup()

    def _loadPlugin(self):
        """
        Load the state of the plugin.
        """
        self.load(self.ColorMapList)
        if not self.hasState(self.ColorMapList):
            self._setColorMapListToDefault()

        self.load(self.ColorBarToggle)
        if not self.hasState(self.ColorBarToggle):
            self.ColorBarToggle.blockSignals(True)
            self.ColorBarToggle.setChecked(True)
            self.ColorBarToggle.blockSignals(False)

        self.load(self.ColorMapReverse)
        if not self.hasState(self.ColorMapReverse):
            self.ColorMapReverse.setChecked(False)

        self.load(self.ColorBarRangeType)
        if not self.hasState(self.ColorBarRangeType):
            self.ColorBarRangeType.setChecked(True)

        self.load(self.RangeMinimumMode)
        if not self.hasState(self.RangeMinimumMode):
            self.RangeMinimumMode.setChecked(False)

        self.load(self.RangeMaximumMode)
        if not self.hasState(self.RangeMaximumMode):
            self.RangeMaximumMode.setChecked(False)

        if self.RangeMinimumMode.isChecked():
            self.load(self.RangeMinimum)

        if self.RangeMaximumMode.isChecked():
            self.load(self.RangeMaximum)

    def onUpdateWindow(self, window, reader, result):
        """
        Whenever the window is update the default ranges must be recalculated.
        """
        rng = result.getRange(local=self.ColorBarRangeType.isChecked())
        self._setDefaultLimitHelper(self.RangeMinimumMode, self.RangeMinimum, rng[0])
        self._setDefaultLimitHelper(self.RangeMaximumMode, self.RangeMaximum, rng[1])

    def onSetupWindow(self, window):
        """
        Store the RenderWindow for adding and removing the colorbar
        """
        self._window = window
        self.updateColorbarOptions()

    def onSetupResult(self, result):
        """
        Add the colorbar after the result is changed.
        """
        self._result = result
        self.updateColorbarOptions()

    def onResetWindow(self):
        """
        Remove stored chigger objects.
        """
        self._window = None
        self._result = None

    def onColorbarOptionsChanged(self, options):
        """
        Attached to BackgroundPlugin.
        """
        if self._colorbar is not None:
            self._colorbar.setOptions(**options)

    def updateOptions(self):
        """
        Update result/reader options for this widget.
        """
        self.updateResultOptions()
        self.updateColorbarOptions()

    def updateResultOptions(self):
        """
        Update the ExodusResult options.
        """
        if (self._variable is None):# or (self._result is None):
            self.setEnabled(False)
            return
        else:
            self.setEnabled(True)

        # ExodusResult options
        result_options = dict()

        # Min./Max. range
        result_options['min'] = self._setLimitHelper(self.RangeMinimumMode, self.RangeMinimum)
        result_options['max'] = self._setLimitHelper(self.RangeMaximumMode, self.RangeMaximum)

        # Colormap
        result_options['cmap'] = str(self.ColorMapList.currentText())
        result_options['cmap_reverse'] = self.ColorMapReverse.isChecked()
        result_options['local_range'] = self.ColorBarRangeType.isChecked()

        # Components
        result_options['component'] = self._component

        # Colorbar options
        self.resultOptionsChanged.emit(result_options)

    def updateColorbarOptions(self):
        """
        Add/remove the colorbar object.
        """
        if (self._window is None) or (self._result is None):
            return

        visible = self.ColorBarToggle.isChecked()
        if visible:
            if self._colorbar is None:
                self._colorbar = chigger.exodus.ExodusColorBar(self._result, layer=3)

            if self._colorbar not in self._window:
                self._window.append(self._colorbar)

        elif (self._colorbar in self._window) and (self._colorbar is not None):
            self._window.remove(self._colorbar)
            self._colorbar = None

    @staticmethod
    def _setLimitHelper(mode, qobject):
        """
        Helper for setting min/max to a user defined value.
        """
        if mode.isChecked():
            qobject.setEnabled(True)
            qobject.setStyleSheet('color:#000000')
            try:
                value = float(qobject.text())
                return value
            except ValueError:
                qobject.setStyleSheet('color:#ff0000')
        return None

    @staticmethod
    def _setDefaultLimitHelper(mode, qobject, default):
        """
        Helper for setting the default min/max values.
        """
        if not mode.isChecked():
            qobject.setEnabled(False)
            qobject.setText(str(default))
            qobject.setStyleSheet('color:#8C8C8C')

    def _setupRangeMinimumMode(self, qobject):
        """
        Set the options for range modes for the minimum control.
        """
        qobject.clicked.connect(self._callbackRangeMinimumMode)

    def _callbackRangeMinimumMode(self):
        """
        Callback for min. range mode toggle.
        """
        if self.RangeMinimumMode.isChecked():
            self.load(self.RangeMinimum)
        self.store(self.RangeMinimumMode)
        self.updateOptions()
        self.windowRequiresUpdate.emit()

    def _setupRangeMinimum(self, qobject):
        """
        Setup the range minimum editing.
        """
        qobject.editingFinished.connect(self._callbackRangeMinimum)

    def _callbackRangeMinimum(self):
        if self.RangeMinimumMode.isChecked():
            self.store(self.RangeMinimum)
        self.updateOptions()
        self.windowRequiresUpdate.emit()

    def _setupRangeMaximumMode(self, qobject):
        """
        Set the options for range modes for the maximum control.
        """
        qobject.clicked.connect(self._callbackRangeMaximumMode)

    def _callbackRangeMaximumMode(self):
        """
        Callback for max. range mode toggle.
        """
        if self.RangeMaximumMode.isChecked():
            self.load(self.RangeMaximum)
        self.store(self.RangeMaximumMode)
        self.updateOptions()
        self.windowRequiresUpdate.emit()

    def _setupRangeMaximum(self, qobject):
        """
        Setup the range minimum editing.
        """
        qobject.editingFinished.connect(self._callbackRangeMaximum)

    def _callbackRangeMaximum(self):
        if self.RangeMaximumMode.isChecked():
            self.store(self.RangeMaximum)
        self.updateOptions()
        self.windowRequiresUpdate.emit()

    def _availableColorMaps(self):
        filenames = glob.glob(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', 'icons', 'colormaps', '*.png')))
        colormaps = {}
        for i in range(len(filenames)):
            name = os.path.basename(filenames[i])[0:-4]
            colormaps[name] = filenames[i]
        return colormaps

    def _setupColorMapList(self, qobject):
        """
        Setup the list of colormaps.
        """
        colormaps = self._availableColorMaps()
        names = sorted(colormaps.keys())
        for i, name in enumerate(names):
            self.ColorMapList.addItem(name)
            self.ColorMapList.setItemIcon(i, QtGui.QIcon(colormaps[name]))

        self._setColorMapListToDefault()
        qobject.currentIndexChanged.connect(self._callbackColorMapList)

    def _setColorMapListToDefault(self):
        """
        Loads the default colormap via the preferences.
        """
        default = self._preferences.value("exodus/defaultColorMap")
        for i in range(self.ColorMapList.count()):
            if default == self.ColorMapList.itemText(i):
                self.ColorMapList.blockSignals(True)
                self.ColorMapList.setCurrentIndex(i)
                self.ColorMapList.blockSignals(False)
                break

    def _callbackColorMapList(self):
        """
        Callback for colormap selection.
        """
        self.store(self.ColorMapList)
        self.updateOptions()
        self.windowRequiresUpdate.emit()

    def _setupColorMapReverse(self, qobject):
        """
        Setup the reverse toggle.
        """
        qobject.stateChanged.connect(self._callbackColorMapReverse)

    def _callbackColorMapReverse(self, value):
        """
        Callback for reverse toggle.
        """
        self.store(self.ColorMapReverse)
        self.updateOptions()
        self.windowRequiresUpdate.emit()

    def _setupColorBarToggle(self, qobject):
        """
        Setup the colorbar toggle.
        """
        qobject.setChecked(True)
        qobject.clicked.connect(self._callbackColorBarToggle)

    def _callbackColorBarToggle(self, value):
        """
        Callback for the colorbar toggle.
        """
        self.store(self.ColorBarToggle)
        self.updateOptions()
        self.windowRequiresUpdate.emit()

    def _setupColorBarRangeType(self, qobject):
        qobject.setChecked(True)
        qobject.setToolTip("Toggle to use the visible data or all data for computing default range.")
        qobject.stateChanged.connect(self._callbackColorBarRangeType)

    def _callbackColorBarRangeType(self, value):
        self.store(self.ColorBarRangeType)
        self.updateOptions()
        self.windowRequiresUpdate.emit()

    def repr(self):
        output = dict()
        colorbar_options, colorbar_sub_options = self._colorbar.options().toScriptString()
        output['colorbar'] = ['cbar = chigger.exodus.ExodusColorBar(result)']
        output['colorbar'] += ['cbar.setOptions({})'.format(', '.join(colorbar_options))]
        for key, value in colorbar_sub_options.iteritems():
            output['colorbar'] += ['cbar.setOptions({}, {})'.format(repr(key), ', '.join(value))]
        return output

def main(size=None):
    """
    Run the VTKFilePlugin all by its lonesome.
    """
    from peacock.ExodusViewer.ExodusPluginManager import ExodusPluginManager
    from VTKWindowPlugin import VTKWindowPlugin
    from FilePlugin import FilePlugin
    widget = ExodusPluginManager(plugins=[lambda: VTKWindowPlugin(size=size), FilePlugin, ColorbarPlugin])
    widget.show()

    return widget, widget.VTKWindowPlugin

if __name__ == '__main__':
    from peacock.utils import Testing
    app = QtWidgets.QApplication(sys.argv)
    filenames = Testing.get_chigger_input_list('mug_blocks_out.e', 'vector_out.e', 'displace.e')
    widget, _ = main()
    widget.FilePlugin.onSetFilenames(filenames)
    sys.exit(app.exec_())
