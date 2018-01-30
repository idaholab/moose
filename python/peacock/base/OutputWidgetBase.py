#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from PyQt5 import QtCore, QtWidgets
from peacock.utils.TextSubWindow import TextSubWindow
from peacock.utils import WidgetUtils

class OutputWidgetBase(QtWidgets.QWidget):
    """
    Plugin responsible for triggering the creation of png/pdf/py files and live script window.
    """

    write = QtCore.pyqtSignal(str)

    def __init__(self):
        super(OutputWidgetBase, self).__init__()

        self._icon_size = QtCore.QSize(32, 32)

        self.MainLayout = QtWidgets.QHBoxLayout()
        self.MainLayout.setContentsMargins(0, 0, 0, 0)
        self.MainLayout.setSpacing(5)

        self.PythonButton = QtWidgets.QPushButton()
        self.PDFButton = QtWidgets.QPushButton()
        self.PNGButton = QtWidgets.QPushButton()
        self.LiveScriptButton = QtWidgets.QPushButton()
        self.LiveScript = TextSubWindow()

        self.setLayout(self.MainLayout)
        self.setSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)

        self.MainLayout.addWidget(self.PythonButton)
        self.MainLayout.addWidget(self.PDFButton)
        self.MainLayout.addWidget(self.PNGButton)
        self.MainLayout.addWidget(self.LiveScriptButton)

    def updateLiveScriptText(self):
        """
        Updates the live script view.
        """
        if self.LiveScript.isVisible() and hasattr(self, "_plugin_manager"):
            # don't reset the text if it is the same. This allows for easier select/copy
            s = self._plugin_manager.repr()
            if s != self.LiveScript.toPlainText():
                self.LiveScript.setText(s)

    def _setupPythonButton(self, qobject):
        """
        Setup method for python script output button.
        """
        qobject.clicked.connect(self._callbackPythonButton)
        qobject.setIcon(WidgetUtils.createIcon('py.svg'))
        qobject.setIconSize(self._icon_size)
        qobject.setFixedSize(qobject.iconSize())
        qobject.setToolTip("Create python script to reproduce this figure.")
        qobject.setStyleSheet("QPushButton {border:none}")

    def _callbackPythonButton(self):
        """
        Open dialog and write script.
        """
        dialog = QtWidgets.QFileDialog()
        dialog.setWindowTitle('Write Python Script')
        dialog.setNameFilter('Python Files (*.py)')
        dialog.setFileMode(QtWidgets.QFileDialog.AnyFile)
        dialog.setAcceptMode(QtWidgets.QFileDialog.AcceptSave)
        dialog.setOption(QtWidgets.QFileDialog.DontUseNativeDialog)

        if dialog.exec_() == QtWidgets.QDialog.Accepted:
            filename = str(dialog.selectedFiles()[0])
            self.write.emit(filename)

    def _setupPDFButton(self, qobject):
        """
        Setup method pdf image output.
        """
        qobject.clicked.connect(self._callbackPDFButton)
        qobject.setIcon(WidgetUtils.createIcon('pdf.svg'))
        qobject.setIconSize(self._icon_size)
        qobject.setFixedSize(qobject.iconSize())
        qobject.setToolTip("Create a pdf file of the current figure.")
        qobject.setStyleSheet("QPushButton {border:none}")

    def _callbackPDFButton(self):
        """
        Write a PDF file of figure.
        """
        dialog = QtWidgets.QFileDialog()
        dialog.setWindowTitle('Write *.pdf of figure')
        dialog.setNameFilter('PDF files (*.pdf)')
        dialog.setFileMode(QtWidgets.QFileDialog.AnyFile)
        dialog.setAcceptMode(QtWidgets.QFileDialog.AcceptSave)
        dialog.setOption(QtWidgets.QFileDialog.DontUseNativeDialog)

        if dialog.exec_() == QtWidgets.QDialog.Accepted:
            filename = str(dialog.selectedFiles()[0])
            self.write.emit(filename)

    def _setupPNGButton(self, qobject):
        """
        Setup method png image output.
        """
        qobject.clicked.connect(self._callbackPNGButton)
        qobject.setIcon(WidgetUtils.createIcon('png.svg'))
        qobject.setIconSize(self._icon_size)
        qobject.setFixedSize(qobject.iconSize())
        qobject.setToolTip("Create a png file of the current figure.")
        qobject.setStyleSheet("QPushButton {border:none}")

    def _callbackPNGButton(self):
        """
        Write a png file of figure.
        """
        dialog = QtWidgets.QFileDialog()
        dialog.setWindowTitle('Write *.png of figure')
        dialog.setNameFilter('PNG files (*.png)')
        dialog.setFileMode(QtWidgets.QFileDialog.AnyFile)
        dialog.setAcceptMode(QtWidgets.QFileDialog.AcceptSave)
        dialog.setOption(QtWidgets.QFileDialog.DontUseNativeDialog)
        dialog.setDefaultSuffix("png")

        if dialog.exec_() == QtWidgets.QDialog.Accepted:
            filename = str(dialog.selectedFiles()[0])
            self.write.emit(filename)

    def _setupLiveScriptButton(self, qobject):
        """
        Setup method png image output.
        """
        qobject.clicked.connect(self._callbackLiveScriptButton)
        qobject.setIcon(WidgetUtils.createIcon('script.svg'))
        qobject.setIconSize(self._icon_size)
        qobject.setFixedSize(qobject.iconSize())
        qobject.setToolTip("Show the current python script.")
        qobject.setStyleSheet("QPushButton {border:none}")

    def _callbackLiveScriptButton(self):
        """
        Write a png file of figure.
        """
        self.LiveScript.show()
        self.updateLiveScriptText()

    def _setupLiveScript(self, qobject):
        """
        Setup for the script text window.
        """
        self.setSizePolicy(QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Preferred)
        qobject.setReadOnly(True)
