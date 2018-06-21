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
import mooseutils
import chigger
import peacock
from ExodusPlugin import ExodusPlugin

class ContourPlugin(peacock.base.PeacockCollapsibleWidget, ExodusPlugin):
    """
    Widget for enabling and controlling contours.
    """

    #: Emitted when the window needs updating
    windowRequiresUpdate = QtCore.pyqtSignal()

    #: Emitted when the contour buttons is clicked
    contourClicked = QtCore.pyqtSignal(bool)

    #: pyqtSignal: Emitted when the chigger objects options are changed
    resultOptionsChanged = QtCore.pyqtSignal(dict)

    def __init__(self):
        peacock.base.PeacockCollapsibleWidget.__init__(self, collapsible_layout=QtWidgets.QVBoxLayout)
        ExodusPlugin.__init__(self)

        self.setTitle('Contours')
        self.setProperty('cache', ['Variable'])

        self.MainLayout = self.collapsibleLayout()

        self.CountLayout = QtWidgets.QHBoxLayout()
        self.LevelLayout = QtWidgets.QHBoxLayout()

        self.ContourToggle = QtWidgets.QCheckBox('Enable Contours')
        self.ContourCountLabel = QtWidgets.QLabel("Count:")
        self.ContourCount = QtWidgets.QSpinBox()
        self.ContourLevelsLabel = QtWidgets.QLabel("Levels:")
        self.ContourLevels = QtWidgets.QLineEdit()

        self.MainLayout.addWidget(self.ContourToggle)
        self.CountLayout.addWidget(self.ContourCountLabel)
        self.CountLayout.addWidget(self.ContourCount)
        self.CountLayout.addStretch()

        self.LevelLayout.addWidget(self.ContourLevelsLabel)
        self.LevelLayout.addWidget(self.ContourLevels)

        self.MainLayout.addLayout(self.CountLayout)
        self.MainLayout.addLayout(self.LevelLayout)

        self._contour = chigger.filters.ContourFilter()
        self.setup()
        self.setCollapsed(True)

    def onSetFilenames(self, *args):
        """
        The contour option should be disabled initially, because it depends on variable type.
        """
        self.setActive()

    def setActive(self):
        status = False
        if self._result:
            varinfo = self._result[0].getCurrentVariableInformation()
            if (self._variable) and (varinfo.object_type == chigger.exodus.ExodusReader.NODAL):
                status = True

        self.ContourToggle.setEnabled(status)
        self.setEnabled(status)
        return status

    def onFileChanged(self, *args):
        """
        If the file changes, update the contours.
        """
        super(ContourPlugin, self).onFileChanged(*args)
        self.contour()

    def onVariableChanged(self, *args):
        """
        When a variable changes, load the state of the clip.
        """
        super(ContourPlugin, self).onVariableChanged(*args)
        self.load(self.stateKey(self._variable), 'Variable')
        if self._result:
            self.contour()

    def contour(self):
        """
        Called when contours are toggled or the count or levels are changed.
        """
        # Disable for non-nodal variables
        active = self.setActive()
        if not active:
            return

        # Set the visibility of the contours
        checked = self.ContourToggle.isChecked()
        self.ContourLevels.setEnabled(checked)
        self.ContourCount.setEnabled(checked)
        filters = self._result.getOption('filters')

        # If visible setup the contour interval/count
        if checked:
            options = dict()
            text = self.ContourLevels.text()
            if len(text) > 0:
                try:
                    options['levels'] = [float(item) for item in re.split('[;,\s]', str(text))]
                    self.ContourCount.setEnabled(False)
                except:
                    mooseutils.mooseError("Failed to convert supplied data to valid numeric contour levels.")
                    self.ContourCount.setEnabled(True)
            else:
                self.ContourCount.setEnabled(True)
                options['levels'] = None
                options['count'] = int(self.ContourCount.value())

            self._contour.setOptions(**options)

            if self._contour not in filters:
                filters.append(self._contour)

        else:
            if self._contour in filters:
                filters.remove(self._contour)

        # Emit the changes
        if self._variable:
            self.store(self.stateKey(self._variable), 'Variable')
        self.resultOptionsChanged.emit({'filters':filters})
        self.windowRequiresUpdate.emit()
        self.contourClicked.emit(checked)

    def repr(self):
        """
        Return python scripting content.
        """
        output = dict()
        if self.ContourToggle.isChecked():
            options, sub_options = self._contour.options().toScriptString()
            output['filters'] = ['contour = chigger.filters.ContourFilter()']
            output['filters'] += ['contour.setOptions({})'.format(', '.join(options))]
            for key, value in sub_options.iteritems():
                output['filters'] += ['contour.setOptions({}, {})'.format(repr(key), ', '.join(value))]

        return output

    def _setupContourToggle(self, qobject):
        """
        Setup method for the contour toggle.
        """
        qobject.clicked.connect(self.contour)

    def _setupContourCount(self, qobject):
        """
        Setup method for the contour counter spinbox.
        """
        qobject.setValue(10)
        qobject.setEnabled(False)
        qobject.valueChanged.connect(self.contour)

    def _setupContourLevels(self, qobject):
        """
        Setup method for the contour level edit.
        """
        qobject.editingFinished.connect(self.contour)


def main(size=None):
    """
    Run the ContourPlugin all by its lonesome.
    """
    from peacock.ExodusViewer.ExodusPluginManager import ExodusPluginManager
    from peacock.ExodusViewer.plugins.VTKWindowPlugin import VTKWindowPlugin
    from peacock.ExodusViewer.plugins.VariablePlugin import VariablePlugin
    widget = ExodusPluginManager(plugins=[lambda: VTKWindowPlugin(size=size), ContourPlugin, VariablePlugin])
    widget.show()

    return widget, widget.VTKWindowPlugin

if __name__ == '__main__':
    from peacock.utils import Testing
    app = QtWidgets.QApplication(sys.argv)
    filename = Testing.get_chigger_input('mug_blocks_out.e')
    widget, window = main()
    widget.initialize([filename])
    window.onResultOptionsChanged({'variable':'diffused'})
    window.onWindowRequiresUpdate()
    sys.exit(app.exec_())
