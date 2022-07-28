#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import sys
from PyQt5 import QtCore, QtWidgets
import chigger
from .ExodusPlugin import ExodusPlugin

class ClipPlugin(QtWidgets.QGroupBox, ExodusPlugin):
    """
    Controls for clipping data in x,y,z direction.
    """

    #: Emitted when the window needs updating
    windowRequiresUpdate = QtCore.pyqtSignal()

    #: pyqtSignal: Emitted when the chigger objects options are changed
    resultOptionsChanged = QtCore.pyqtSignal(dict)

    #: pyqtSignal: Emitted when adding a filter
    addFilter = QtCore.pyqtSignal(chigger.filters.ChiggerFilterBase)

    #: pyqtSignal: Emitted when removing a filter
    removeFilter = QtCore.pyqtSignal(chigger.filters.ChiggerFilterBase)

    def __init__(self):
        super(ClipPlugin, self).__init__()

        # Member variables
        self._origin = None
        self._increments = 41

        # The chigger clipper object
        self._clipper = chigger.filters.PlaneClipper()

        # Setup this widget widget
        self.setTitle('Clip')
        self.setCheckable(True)
        self.setChecked(False)
        self.MainLayout = QtWidgets.QHBoxLayout(self)
        self.MainLayout.setSpacing(0)
        self.MainLayout.setContentsMargins(0,0,0,0)

        self.ClipDirection = QtWidgets.QComboBox()
        self.ClipSlider = QtWidgets.QSlider()

        self.MainLayout.addWidget(self.ClipDirection)
        self.MainLayout.addSpacing(10)
        self.MainLayout.addWidget(self.ClipSlider)
        self.clicked.connect(self._callbackClicked)

        self.setup()
        self.store(key='default')

    def _loadPlugin(self):
        """
        Helper for loading plugin state.
        """
        self.load()
        if not self.hasState():
            self.setChecked(False)

    def repr(self):
        """
        Return python scripting content.
        """
        output = dict()
        if self.isChecked():
            options, sub_options = self._clipper.options().toScriptString()
            output['filters'] = ['clipper = chigger.filters.PlaneClipper()']
            output['filters'] += ['clipper.setOptions({})'.format(', '.join(options))]
            for key, value in sub_options.items():
                output['filters'] += ['clipper.setOptions({}, {})'.format(repr(key), ', '.join(value))]

        return output

    def updateOptions(self):
        """
        Update the clipping settings.
        """

        checked = self.isChecked()
        self.ClipDirection.setEnabled(checked)
        self.ClipSlider.setEnabled(checked)

        if checked:
            index = self.ClipDirection.currentIndex()
            normal = [0, 0, 0]
            normal[index] = 1

            origin = [0.5, 0.5, 0.5]
            origin[index] = self.ClipSlider.sliderPosition()/float(self._increments-1)

            self._clipper.setOptions(normal=normal, origin=origin)
            self.addFilter.emit(self._clipper)
        else:
            self.removeFilter.emit(self._clipper)

    def _callbackClicked(self):
        """
        Setup method for the clip toggle.
        """
        self.store()
        self.updateOptions()
        self.windowRequiresUpdate.emit()

    def _setupClipDirection(self, qobject):
        """
        Setup method for clip direction selection.
        """
        qobject.addItem('X')
        qobject.addItem('Y')
        qobject.addItem('Z')
        qobject.setEnabled(False)
        qobject.currentIndexChanged.connect(self._callbackClipDirection)

    def _callbackClipDirection(self):
        """
        Callback for when clip direction is altered.
        """
        self.store(self.ClipDirection)
        key = (self._filename, self._variable, self._component, self.ClipDirection.currentIndex())
        self.load(self.ClipSlider, key=key)
        self.updateOptions()
        self.windowRequiresUpdate.emit()

    def _setupClipSlider(self, qobject):
        """
        Setup method for clip origin selection.
        """
        qobject.setOrientation(QtCore.Qt.Horizontal)
        qobject.setRange(0, self._increments)
        qobject.setSliderPosition(self._increments//2)
        qobject.setProperty('cache', ['ClipDirection'])
        qobject.valueChanged.connect(self._callbackClipSlider)
        qobject.setEnabled(False)

    def _callbackClipSlider(self, value):
        """
        Callback for slider.
        """
        self.store(self.ClipSlider)
        key = (self._filename, self._variable, self._component, self.ClipDirection.currentIndex())
        self.store(self.ClipSlider, key=key)
        self.updateOptions()
        self.windowRequiresUpdate.emit()

def main(size=None):
    """
    Run the ClipPlugin all by its lonesome.
    """
    from peacock.ExodusViewer.ExodusPluginManager import ExodusPluginManager
    from peacock.ExodusViewer.plugins.VTKWindowPlugin import VTKWindowPlugin
    from peacock.ExodusViewer.plugins.FilePlugin import FilePlugin
    widget = ExodusPluginManager(plugins=[lambda: VTKWindowPlugin(size=size), FilePlugin, ClipPlugin])
    widget.show()

    return widget, widget.VTKWindowPlugin

if __name__ == '__main__':
    from peacock.utils import Testing
    app = QtWidgets.QApplication(sys.argv)
    filenames = Testing.get_chigger_input_list('mug_blocks_out.e', 'vector_out.e')
    widget, window = main(size=[600,600])
    widget.FilePlugin.onSetFilenames(filenames)
    sys.exit(app.exec_())
