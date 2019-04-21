#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import sys
from PyQt5 import QtCore, QtGui, QtWidgets
from .ExodusPlugin import ExodusPlugin

class BackgroundPlugin(QtWidgets.QWidget, ExodusPlugin):
    """
    Plugin responsible for background colors.

    This plugin only contains menu items, see the addToMenu method.
    """

    #: Emitted when the window needs updated.
    windowRequiresUpdate = QtCore.pyqtSignal()

    #: pyqtSignal: Emitted when the chigger objects options are changed
    windowOptionsChanged = QtCore.pyqtSignal(dict)

    #: pyqtSignal: Emitted when the colorbar options are changed
    colorbarOptionsChanged = QtCore.pyqtSignal(dict)

    #: pyqtSignal: Emitted when the result options are changed
    resultOptionsChanged = QtCore.pyqtSignal(dict)

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
        self.hide()
        self.GradientToggle = None
        self.BlackPreset = None
        self.WhitePreset = None
        self.GradientTopColor = None
        self.GradientBottomColor = None
        self.SolidColor = None
        self._set_result_color = kwargs.pop('set_result_color', False)
        self._gradient_state = True
        self._black_font_state = False

        # Default colors
        self._top = QtGui.QColor(self._preferences.value("exodus/gradientTopColor"))
        self._bottom = QtGui.QColor(self._preferences.value("exodus/gradientBottomColor"))
        self._solid = QtGui.QColor(self._preferences.value("exodus/solidBackgroundColor"))

        # Setup this widget
        self.setup()

    def _callbackGradientTopColor(self):
        """
        Callback of for selecting top color of gradient
        """
        dialog = QtWidgets.QColorDialog()
        c = dialog.getColor(initial=self._top, title='Select top gradient color')
        if c.isValid():
            self._top = c
            self.updateOptions()

    def _prefCallbackGradientTopColor(self, value):
        """
        Updates top color when preference saved.
        """
        self._top = QtGui.QColor(value)
        self.updateOptions()
        self.windowRequiresUpdate.emit()

    def _callbackGradientBottomColor(self):
        """
        Callback for selecting bottom color of gradient
        """
        dialog = QtWidgets.QColorDialog()
        c = dialog.getColor(initial=self._bottom, title='Select bottom gradient color')
        if c.isValid():
            self._bottom = c
            self.updateOptions()

    def _prefCallbackGradientBottomColor(self, value):
        """
        Updates bottom color when preference saved.
        """
        self._bottom = QtGui.QColor(value)
        self.updateOptions()
        self.windowRequiresUpdate.emit()

    def _callbackSolidColor(self):
        """
        Callback for selecting solid color.
        """
        dialog = QtWidgets.QColorDialog()
        c = dialog.getColor(initial=self._solid, title='Select solid color')
        if c.isValid():
            self._solid = c
            self.updateOptions()

    def _prefCallbackGradientSolidColor(self, value):
        """
        Updates solid color when preference saved.
        """
        self._solid = QtGui.QColor(value)
        self.updateOptions()
        self.windowRequiresUpdate.emit()

    def _prefCallbackBackgroundGradient(self, value):
        """
        Updates top color when preference saved.
        """
        self.GradientToggle.setChecked(value)
        self.updateOptions()
        self.windowRequiresUpdate.emit()

    def onSetupWindow(self, *args):
        """
        Update RenderWindow options.
        """
        self.updateWindowOptions()

    def onSetupResult(self, result):
        """
        Update ExodusResult options.
        """
        self.updateResultOptions()

    def onSetupColorbar(self, colorbar):
        """
        Update ExodusColorbar options.
        """
        self.ColorbarBlackFontToggle.setVisible(colorbar[0].getOption('visible'))
        self.updateOptions()

    def onSetEnableWidget(self, value):
        """
        Enable/disable the menu items.
        """
        super(BackgroundPlugin, self).onSetEnableWidget(value)
        self.GradientToggle.setEnabled(value)
        self.BlackPreset.setEnabled(value)
        self.WhitePreset.setEnabled(value)
        self.ColorbarBlackFontToggle.setEnabled(value)
        self.TopGradientColor.setEnabled(value)
        self.BottomGradientColor.setEnabled(value)
        self.SolidColor.setEnabled(value)

    def addToMenu(self, menu):
        """
        Create a toggle for the background color.
        """
        submenu = menu.addMenu('Background')

        toggle = self._preferences.value("exodus/backgroundGradient")
        self.GradientToggle = submenu.addAction("Gradient")
        self.GradientToggle.setCheckable(True)
        self.GradientToggle.setChecked(toggle)
        self.GradientToggle.toggled.connect(self._callbackGradientToggle)

        submenu.addSeparator()
        self.BlackPreset = submenu.addAction('Black (preset)')
        self.BlackPreset.setCheckable(True)
        self.BlackPreset.setChecked(False)
        self.BlackPreset.toggled.connect(self._callbackBlackPreset)

        self.WhitePreset = submenu.addAction('White (preset)')
        self.WhitePreset.setCheckable(True)
        self.WhitePreset.setChecked(False)
        self.WhitePreset.toggled.connect(self._callbackWhitePreset)

        submenu.addSeparator()
        self.ColorbarBlackFontToggle = submenu.addAction("Use Black Font/Mesh")
        self.ColorbarBlackFontToggle.setCheckable(True)
        self.ColorbarBlackFontToggle.setChecked(False)
        self.ColorbarBlackFontToggle.toggled.connect(self._callbackColorbarBlackFontToggle)

        submenu.addSeparator()
        self.TopGradientColor = submenu.addAction("Select Top Gradient Color")
        self.TopGradientColor.triggered.connect(self._callbackGradientTopColor)

        self.BottomGradientColor = submenu.addAction("Select Bottom Gradient Color")
        self.BottomGradientColor.triggered.connect(self._callbackGradientBottomColor)

        self.SolidColor = submenu.addAction("Select Solid Color")
        self.SolidColor.triggered.connect(self._callbackSolidColor)

        self.onSetEnableWidget(False)

    def updateResultOptions(self):
        """
        Apply ExodusResult options.
        """
        if self.ColorbarBlackFontToggle.isChecked() and self._set_result_color:
            self.resultOptionsChanged.emit({'color':[0,0,0]})
        elif self._set_result_color:
            self.resultOptionsChanged.emit({'color':[1,1,1]})

    def updateColorbarOptions(self):
        """
        Apply the ExodusColorbar options.
        """
        if self.ColorbarBlackFontToggle.isChecked():
            self.colorbarOptionsChanged.emit({'primary':dict(font_color=[0,0,0])})
        else:
            self.colorbarOptionsChanged.emit({'primary':dict(font_color=[1,1,1])})

    def updateWindowOptions(self):
        """
        Apply the RenderWindow options.
        """
        if self.GradientToggle.isChecked():
            top = self._top.getRgb()
            bottom = self._bottom.getRgb()
            background = [bottom[0]/255., bottom[1]/255., bottom[2]/255.]
            background2 = [top[0]/255., top[1]/255., top[2]/255.]
        elif self.BlackPreset.isChecked():
            background = [0, 0, 0]
            background2 = None

        elif self.WhitePreset.isChecked():
            background = [1, 1, 1]
            background2 = None

        else:
            solid = self._solid.getRgb()
            background = [solid[0]/255., solid[1]/255., solid[2]/255.]
            background2 = None

        self.windowOptionsChanged.emit({'background':background,
                                        'background2':background2,
                                        'gradient_background':self.GradientToggle.isChecked()})

    def updateOptions(self):
        """
        Apply options to all chigger objects.
        """
        self.updateResultOptions()
        self.updateColorbarOptions()
        self.updateWindowOptions()

    def _setupBackgroundSelect(self, qobject):
        """
        Setup the background toggle options.
        """
        qobject.addItem('Gradient')
        qobject.addItem('Black')
        qobject.addItem('White')
        qobject.addItem('Solid (custom)')
        qobject.currentIndexChanged.connect(self._callbackBackgroundSelect)

    def _callbackGradientToggle(self, value):
        """
        Called when the gradient toggle is checked/Unchecked.
        """
        self.GradientToggle.setChecked(value)
        self._gradient_state = value
        if value:
            self.BlackPreset.blockSignals(True)
            self.BlackPreset.setChecked(False)
            self.BlackPreset.blockSignals(False)

            self.WhitePreset.blockSignals(True)
            self.WhitePreset.setChecked(False)
            self.WhitePreset.blockSignals(False)

            self.ColorbarBlackFontToggle.blockSignals(True)
            self.ColorbarBlackFontToggle.setChecked(False)
            self.ColorbarBlackFontToggle.blockSignals(False)

        self.updateOptions()
        self.windowRequiresUpdate.emit()

    def _callbackBlackPreset(self, value):
        """
        Called when the black preset is toggled.
        """
        self.BlackPreset.setChecked(value)
        if value:
            self.GradientToggle.blockSignals(True)
            self.GradientToggle.setChecked(False)
            self.GradientToggle.blockSignals(False)

            self.WhitePreset.blockSignals(True)
            self.WhitePreset.setChecked(False)
            self.WhitePreset.blockSignals(False)

            self.ColorbarBlackFontToggle.blockSignals(True)
            self.ColorbarBlackFontToggle.setChecked(False)
            self.ColorbarBlackFontToggle.blockSignals(False)

        else:
            self.GradientToggle.blockSignals(True)
            self.GradientToggle.setChecked(self._gradient_state)
            self.GradientToggle.blockSignals(False)

            self.ColorbarBlackFontToggle.blockSignals(True)
            self.ColorbarBlackFontToggle.setChecked(self._black_font_state)
            self.ColorbarBlackFontToggle.blockSignals(False)

        self.updateOptions()
        self.windowRequiresUpdate.emit()

    def _callbackWhitePreset(self, value):
        """
        Called when the white preset is toggled.
        """
        self.WhitePreset.setChecked(value)
        if value:
            self.GradientToggle.blockSignals(True)
            self.GradientToggle.setChecked(False)
            self.GradientToggle.blockSignals(False)

            self.BlackPreset.blockSignals(True)
            self.BlackPreset.setChecked(False)
            self.BlackPreset.blockSignals(False)

            self.ColorbarBlackFontToggle.blockSignals(True)
            self.ColorbarBlackFontToggle.setChecked(True)
            self.ColorbarBlackFontToggle.blockSignals(False)

        else:
            self.GradientToggle.blockSignals(True)
            self.GradientToggle.setChecked(self._gradient_state)
            self.GradientToggle.blockSignals(False)

            self.ColorbarBlackFontToggle.blockSignals(True)
            self.ColorbarBlackFontToggle.setChecked(self._black_font_state)
            self.ColorbarBlackFontToggle.blockSignals(False)

        self.updateOptions()
        self.windowRequiresUpdate.emit()

    def _callbackColorbarBlackFontToggle(self, value):
        """
        Called when the drop down black font option is toggled.
        """
        self._black_font_state = value
        self.ColorbarBlackFontToggle.setChecked(value)
        self.updateOptions()
        self.windowRequiresUpdate.emit()

def main(size=None):
    """
    Run the BackgroundPlugin all by its lonesome.
    """
    from peacock.ExodusViewer.ExodusPluginManager import ExodusPluginManager
    from peacock.ExodusViewer.plugins.VTKWindowPlugin import VTKWindowPlugin
    from peacock.ExodusViewer.plugins.ColorbarPlugin import ColorbarPlugin
    widget = ExodusPluginManager(plugins=[lambda: VTKWindowPlugin(size=size),
                                          BackgroundPlugin,
                                          ColorbarPlugin])
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
