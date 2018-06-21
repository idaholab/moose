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
import peacock
from ExodusPlugin import ExodusPlugin
from peacock.utils import WidgetUtils

class ClipPlugin(peacock.base.PeacockCollapsibleWidget, ExodusPlugin):
    """
    Controls for clipping data in x,y,z direction.
    """

    #: Emitted when the window needs updating
    windowRequiresUpdate = QtCore.pyqtSignal()

    #: pyqtSignal: Emitted when the chigger objects options are changed
    resultOptionsChanged = QtCore.pyqtSignal(dict)

    def __init__(self, **kwargs):
        peacock.base.PeacockCollapsibleWidget.__init__(self)
        ExodusPlugin.__init__(self, **kwargs)

        # Member variables
        self._origin = None
        self._increments = 41

        # The chigger clipper object
        self._clipper = chigger.filters.PlaneClipper()

        # Setup this widget widget
        self.setTitle('Clipping')

        self.MainLayout = self.collapsibleLayout()

        self.ClipToggle = QtWidgets.QCheckBox()

        self.ClipDirection = QtWidgets.QComboBox()
        self.ClipSlider = QtWidgets.QSlider()

        self.MainLayout.addWidget(self.ClipToggle)
        self.MainLayout.addWidget(self.ClipDirection)
        self.MainLayout.addWidget(self.ClipSlider)

        self.setup()
        self.setCollapsed(True)

    def repr(self):
        """
        Return python scripting content.
        """
        output = dict()
        if self.ClipToggle.isChecked():
            options, sub_options = self._clipper.options().toScriptString()
            output['filters'] = ['clipper = chigger.filters.PlaneClipper()']
            output['filters'] += ['clipper.setOptions({})'.format(', '.join(options))]
            for key, value in sub_options.iteritems():
                output['filters'] += ['clipper.setOptions({}, {})'.format(repr(key), ', '.join(value))]

        return output

    def onVariableChanged(self, *args):
        """
        When a variable changes, load the state of the clip.
        """
        if self.isEnabled():
            self.store(self.stateKey(self._variable), 'Variable')
        super(ClipPlugin, self).onVariableChanged(*args)
        self.load(self.stateKey(self._variable), 'Variable')

    def clip(self):
        """
        Update the clipping settings.
        """

        # Update visibility status
        checked = self.ClipToggle.isChecked()
        self.ClipDirection.setEnabled(checked)
        self.ClipSlider.setEnabled(checked)
        filters = []
        if self._result:
            filters = self._result.getOption('filters')

        # Clipping
        if checked:
            # Get the current clip axis index
            index = self.ClipDirection.currentIndex()

            normal = [0, 0, 0]
            normal[index] = 1

            origin = [0.5, 0.5, 0.5]
            origin[index] = self.ClipSlider.sliderPosition()/float(self._increments-1)

            self._clipper.setOptions(normal=normal, origin=origin)
            if self._clipper not in filters:
                filters.append(self._clipper)

        else:
            if self._clipper in filters:
                filters.remove(self._clipper)

        self.resultOptionsChanged.emit({'filters':filters})
        self.windowRequiresUpdate.emit()

    def _setupClipToggle(self, qobject):
        """
        Setup method for the clip toggle.
        """
        qobject.stateChanged.connect(lambda val: self.clip())

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
        index = self.ClipDirection.currentIndex()
        WidgetUtils.loadWidget(self.ClipSlider, self.stateKey(index), 'ClipDirection')
        self.clip()

    def _setupClipSlider(self, qobject):
        """
        Setup method for clip origin selection.
        """
        qobject.setOrientation(QtCore.Qt.Horizontal)
        qobject.setRange(0, self._increments)
        qobject.setSliderPosition(self._increments/2)
        qobject.setProperty('cache', ['ClipDirection'])
        qobject.valueChanged.connect(self._callbackClipSlider)

        # Store the initial state of the slide for index 0, why?
        #
        # Perform the following:
        #   (1) Clipping enabled
        #   (2) Change the axis to 'y'
        #   (3) Move the slider
        #   (4) Change the axis back to 'x'
        #
        # Without this 'x' would be at the 'y' position instead of at the midpoint.
        qobject.setEnabled(True)
        WidgetUtils.storeWidget(qobject, self.stateKey(0), 'ClipDirection')
        qobject.setEnabled(False)

    def _callbackClipSlider(self, value):
        """
        Callback for slider.
        """
        index = self.ClipDirection.currentIndex()
        WidgetUtils.storeWidget(self.ClipSlider, self.stateKey(index), 'ClipDirection')
        self.clip()

def main(size=None):
    """
    Run the ClipPlugin all by its lonesome.
    """
    from peacock.ExodusViewer.ExodusPluginManager import ExodusPluginManager
    from peacock.ExodusViewer.plugins.VTKWindowPlugin import VTKWindowPlugin
    widget = ExodusPluginManager(plugins=[lambda: VTKWindowPlugin(size=size), ClipPlugin])
    widget.ClipPlugin.setCollapsed(False)
    widget.show()

    return widget, widget.VTKWindowPlugin

if __name__ == '__main__':
    from peacock.utils import Testing
    app = QtWidgets.QApplication(sys.argv)
    filename = Testing.get_chigger_input('mug_blocks_out.e')
    widget, window = main()
    widget.onFileChanged(filename)
    sys.exit(app.exec_())
