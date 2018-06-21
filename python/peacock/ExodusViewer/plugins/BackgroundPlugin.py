#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import sys
import chigger
from PyQt5 import QtCore, QtGui, QtWidgets
import peacock
from ExodusPlugin import ExodusPlugin

class BackgroundPlugin(peacock.base.PeacockCollapsibleWidget, ExodusPlugin):
    """
    Plugin responsible for background and labels
    """

    #: Emitted when the window needs updated.
    windowRequiresUpdate = QtCore.pyqtSignal()

    #: pyqtSignal: Emitted when the chigger objects options are changed
    windowOptionsChanged = QtCore.pyqtSignal(dict)

    #: pyqtSignal: Add/remove result objects
    appendResult = QtCore.pyqtSignal(chigger.base.ChiggerResultBase)
    removeResult = QtCore.pyqtSignal(chigger.base.ChiggerResultBase)

    def __init__(self, values=True, **kwargs):
        peacock.base.PeacockCollapsibleWidget.__init__(self, collapsible_layout=QtWidgets.QGridLayout)
        ExodusPlugin.__init__(self, **kwargs)

        self._preferences.addBool("exodus/backgroundGradient",
                "Use background gradient",
                True,
                "Turn on/off the background gradient",
                )

        self._preferences.addColor("exodus/gradientTopColor",
                "Background top gradient color",
                QtGui.QColor(111, 111, 111),
                "Set the top gradient color",
                )

        self._preferences.addColor("exodus/gradientBottomColor",
                "Background bottom gradient color",
                QtGui.QColor(180, 180, 180),
                "Set the top gradient color",
                )

        self._preferences.addColor("exodus/solidBackgroundColor",
                "Solid Background color",
                QtGui.QColor(111, 111, 111),
                "Solid Background color",
                )

        # Default colors
        self._top = QtGui.QColor(self._preferences.value("exodus/gradientTopColor"))
        self._bottom = QtGui.QColor(self._preferences.value("exodus/gradientBottomColor"))
        self._solid = QtGui.QColor(self._preferences.value("exodus/solidBackgroundColor"))

        # Setup this widget
        self.MainLayout = self.collapsibleLayout()
        self.setTitle('Background and Labels')
        self.setSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Minimum)

        # Top
        self.TopLabel = QtWidgets.QLabel('Background Top:')
        self.TopButton = QtWidgets.QPushButton()
        self.GradientToggle = QtWidgets.QCheckBox('Background Gradient')
        self.MainLayout.addWidget(self.TopLabel, 1, 0)
        self.MainLayout.addWidget(self.TopButton, 1, 1)
        self.MainLayout.addWidget(self.GradientToggle, 0, 0)

        # Bottom
        self.BottomLabel = QtWidgets.QLabel('Background Bottom:')
        self.BottomButton = QtWidgets.QPushButton()
        self.MainLayout.addWidget(self.BottomLabel, 2, 0)
        self.MainLayout.addWidget(self.BottomButton, 2, 1)

        # Extents
        self.Extents = QtWidgets.QCheckBox('Extents')
        self.MainLayout.addWidget(self.Extents, 0, 3)

        # Storage for Chigger objects that are toggled by this plugin
        self._volume_axes = None
        self.setup()

    def color(self):
        """
        Apply the supplied colors to the window.
        """
        if self._window:
            if self.GradientToggle.isChecked():
                top = self._top.getRgb()

                bottom = self._bottom.getRgb()
                self.BottomButton.setStyleSheet('border:none; background:rgb' + str(bottom))

                background = [bottom[0]/255., bottom[1]/255., bottom[2]/255.]
                background2 = [top[0]/255., top[1]/255., top[2]/255.]
            else:
                top = self._solid.getRgb()
                background = [top[0]/255., top[1]/255., top[2]/255.]
                background2 = None

            self.TopButton.setStyleSheet('border:none; background:rgb' + str(top))
            self.windowOptionsChanged.emit({'background':background, 'background2':background2, 'gradient_background':self.GradientToggle.isChecked()})
            self.windowRequiresUpdate.emit()

    def onWindowCreated(self, *args):
        """
        When the window is created apply the color.
        """
        super(BackgroundPlugin, self).onWindowCreated(*args)
        self.color()

    def _setupGradientToggle(self, qobject):
        """
        Setup method for gradient toggle.
        """
        qobject.setChecked(self._preferences.value("exodus/backgroundGradient"))
        qobject.stateChanged.connect(self._callbackGradientToggle)

    def _callbackGradientToggle(self, value):
        """
        Called when the gradient toggle is checked/Unchecked.
        """

        if value == QtCore.Qt.Checked:
            self.TopLabel.setText('Background Top:')
            self.BottomLabel.setVisible(True)
            self.BottomButton.setVisible(True)
        else:
            self.TopLabel.setText('Background Color:')
            self.BottomLabel.setVisible(False)
            self.BottomButton.setVisible(False)
        self.color()

    def _setupTopButton(self, qobject):
        """
        Setup method for top color button.
        """
        self.TopLabel.setAlignment(QtCore.Qt.AlignRight)
        qobject.setStyleSheet('border:none;')
        qobject.setMaximumWidth(qobject.height())
        qobject.setAutoFillBackground(False)
        qobject.clicked.connect(self._callbackTopButton)

    def _callbackTopButton(self):
        """
        Callback for the top color dialog.
        """
        if self.GradientToggle.isChecked():
            title = 'Select top gradient color'
            default_color = self._top
        else:
            title = 'Select solid color'
            default_color = self._solid

        dialog = QtWidgets.QColorDialog()
        c = dialog.getColor(initial=default_color, title=title)

        if c.isValid():
            if self.GradientToggle.isChecked():
                self._top = c
            else:
                self._solid = c
            self.color()

    def _setupBottomButton(self, qobject):
        """
        Setup the bottom color button.
        """
        self.BottomLabel.setAlignment(QtCore.Qt.AlignRight)
        qobject.setStyleSheet('border:none;')
        qobject.setMaximumWidth(qobject.height())
        qobject.setAutoFillBackground(False)
        qobject.clicked.connect(self._callbackBottomButton)

    def _callbackBottomButton(self):
        """
        Callback for bottom color button.
        """
        dialog = QtWidgets.QColorDialog()
        c = dialog.getColor(initial=self._bottom, title='Select bottom gradient color')
        if c.isValid():
            self._bottom = c
            self.color()

    def _setupExtents(self, qobject):
        """
        Setup method for the extents toggle.
        """
        qobject.stateChanged.connect(self._callbackExtents)

    def _callbackExtents(self, value):
        """
        Enables/disables the extents on the VTKwindow.
        """
        if not self._result:
            return
        elif value == QtCore.Qt.Checked:
            self._volume_axes = chigger.misc.VolumeAxes(self._result)
            self.appendResult.emit(self._volume_axes)
        else:
            self._volume_axes.reset()
            self.removeResult.emit(self._volume_axes)

        self.windowRequiresUpdate.emit()

def main(size=None):
    """
    Run the ContourPlugin all by its lonesome.
    """
    from peacock.ExodusViewer.ExodusPluginManager import ExodusPluginManager
    from peacock.ExodusViewer.plugins.VTKWindowPlugin import VTKWindowPlugin
    widget = ExodusPluginManager(plugins=[lambda: VTKWindowPlugin(size=size), BackgroundPlugin])
    widget.show()

    return widget, widget.VTKWindowPlugin

if __name__ == '__main__':
    from peacock.utils import Testing
    app = QtWidgets.QApplication(sys.argv)
    filename = Testing.get_chigger_input('mug_blocks_out.e')
    widget, window = main()
    widget.onSetFilenames([filename])
    window.onResultOptionsChanged({'variable':'diffused'})
    window.onWindowRequiresUpdate()
    sys.exit(app.exec_())
