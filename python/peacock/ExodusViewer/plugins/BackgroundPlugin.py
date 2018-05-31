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

class BackgroundPlugin(QtWidgets.QWidget, ExodusPlugin):
    """
    Plugin responsible for background and labels
    """

    #: Emitted when the window needs updated.
    windowRequiresUpdate = QtCore.pyqtSignal()

    #: pyqtSignal: Emitted when the chigger objects options are changed
    windowOptionsChanged = QtCore.pyqtSignal(dict)

    def __init__(self, **kwargs):
        super(BackgroundPlugin, self).__init__(**kwargs)

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

        # Background Toggle action (see addToMenu)
        self.GradientToggle = None
        self.setVisible(False)
        self.hide() # this widget only contains menu items

        # Default colors
        self._top = QtGui.QColor(self._preferences.value("exodus/gradientTopColor"))
        self._bottom = QtGui.QColor(self._preferences.value("exodus/gradientBottomColor"))
        self._solid = QtGui.QColor(self._preferences.value("exodus/solidBackgroundColor"))

        # Setup this widget
        self.setup()

    @QtCore.pyqtSlot(chigger.exodus.ExodusResult)
    def onWindowResult(self, *args):
        """
        When the result is created apply the color to the RenderWindow object.
        """
        self.updateOptions()
        self.windowRequiresUpdate.emit()

    def addToMenu(self, menu):
        """
        Create a toggle for the background color.
        """
        self.GradientToggle = menu.addAction("Gradient Background")
        self.GradientToggle.setCheckable(True)
        self.GradientToggle.setChecked(True)
        self.GradientToggle.toggled.connect(self._callbackGradientToggle)
        self.updateOptions()

    def updateOptions(self):
        """
        Apply the supplied colors to the window.
        """
        if self.GradientToggle is None:
            return

        if self.GradientToggle.isChecked():
            top = self._top.getRgb()
            bottom = self._bottom.getRgb()
            background = [bottom[0]/255., bottom[1]/255., bottom[2]/255.]
            background2 = [top[0]/255., top[1]/255., top[2]/255.]
        else:
            top = self._solid.getRgb()
            background = [top[0]/255., top[1]/255., top[2]/255.]
            background2 = None

        self.windowOptionsChanged.emit({'background':background,
                                        'background2':background2,
                                        'gradient_background':self.GradientToggle.isChecked()})

    def _callbackGradientToggle(self, value):
        """
        Called when the gradient toggle is checked/Unchecked.
        """
        self.updateOptions()
        self.windowRequiresUpdate.emit()

def main(size=None):
    """
    Run the BackgroundPlugin all by its lonesome.
    """
    from peacock.ExodusViewer.ExodusPluginManager import ExodusPluginManager
    from peacock.ExodusViewer.plugins.VTKWindowPlugin import VTKWindowPlugin
    widget = ExodusPluginManager(plugins=[lambda: VTKWindowPlugin(size=size), BackgroundPlugin])
    main_window = QtWidgets.QMainWindow()
    main_window.setCentralWidget(widget)
    menubar = main_window.menuBar()
    menubar.setNativeMenuBar(False)
    widget.addToMainMenu(menubar)
    main_window.show()

    return widget, widget.VTKWindowPlugin, main_window

if __name__ == '__main__':
    from peacock.utils import Testing
    app = QtWidgets.QApplication(sys.argv)
    filename = Testing.get_chigger_input('mug_blocks_out.e')
    widget, window, main_window = main()
    window.onSetFilename(filename)
    window.onSetVariable('diffused')
    window.onWindowRequiresUpdate()
    sys.exit(app.exec_())
