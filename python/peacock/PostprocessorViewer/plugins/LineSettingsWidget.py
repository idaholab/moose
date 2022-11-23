#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import sys
from PyQt5 import QtCore, QtWidgets, QtGui
import peacock

class LineSettingsWidget(peacock.base.MooseWidget, QtWidgets.QWidget):
    """
    A widget for containing the appearance of a Line2D object.

    Args:
        variable[str]: The name of the postprocessor that this widget is responsible.

    Kwargs:
        linestyle[str]: A valid matplotlib linestype string.
        linewidth[float]: The width of the line.
        color[list]: RGB color values in a list.
        marker[str]: A valid matplotlib marker style.
        markersize[float]: The size of the marker.
        axis[int]: The axis for the line to appear (left=0, right=1).
    """

    #: pyqtSignal: Emitted when the checkbox is pressed within this widget.
    clicked = QtCore.pyqtSignal()

    #: list: Storage of possible line styles.
    line_styles = ['-', '--', '-.', ':', '']

    #: list: Storage of possible marker styles.
    marker_styles = ['', 'o', 'v', 's', '8', '*', '+', 'd']

    def __init__(self, variable, *args, **kwargs):
        peacock.base.MooseWidget.__init__(self)
        QtWidgets.QWidget.__init__(self, *args)

        # The default settings
        self._settings = dict()
        self._settings['label'] = str(variable)
        self._settings['linestyle'] = kwargs.pop('linestyle', '-')
        self._settings['linewidth'] = kwargs.pop('linewidth', 1)
        self._settings['color'] = kwargs.pop('color', [0,0,1])
        self._settings['marker'] = kwargs.pop('markerstyle', '')
        self._settings['markersize'] = kwargs.pop('markersize', 1)
        self._settings['axis'] = kwargs.pop('axis', 0)

        self.setSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Fixed)

        # Define the main layout
        self.MainLayout = QtWidgets.QHBoxLayout()
        self.MainLayout.setContentsMargins(0, 0, 0, 0);
        self.setLayout(self.MainLayout)
        self.setAutoFillBackground(False)

        # The widgets with the artist settings controls
        self.CheckBox = QtWidgets.QCheckBox(variable)
        self.PlotAxis = QtWidgets.QComboBox()
        self.ColorButton = QtWidgets.QPushButton()
        self.LineStyle = QtWidgets.QComboBox()
        self.LineWidth = QtWidgets.QDoubleSpinBox()
        self.MarkerStyle = QtWidgets.QComboBox()
        self.MarkerSize = QtWidgets.QDoubleSpinBox()

        # Add the control widgets
        self.MainLayout.addWidget(self.ColorButton)
        self.MainLayout.addWidget(self.CheckBox)
        self.MainLayout.addStretch(1)
        self.MainLayout.addWidget(self.PlotAxis)
        self.MainLayout.addWidget(self.LineStyle)
        self.MainLayout.addWidget(self.LineWidth)
        self.MainLayout.addWidget(self.MarkerStyle)
        self.MainLayout.addWidget(self.MarkerSize)

        # Call setup methods
        self.setup()
        self.update()

    def isValid(self):
        """
        Return True if the box is checked and enabled (i.e., it is available for plotting).
        """
        return self.CheckBox.isChecked() and self.CheckBox.isEnabled()

    def axis(self):
        """
        Returns 'left' or 'right' for indicating where the line is to be located.
        """
        return self.PlotAxis.currentText()

    def settings(self):
        """
        Return a copy of the line settings.

        see ArtistGroupWidget
        """
        return self._settings.copy()

    def update(self):
        """
        Emit a signal that the settings for the line have changed.

        see ArtistGroupWidget
        """
        status = self.CheckBox.isChecked()
        self.PlotAxis.setEnabled(status)
        self.ColorButton.setEnabled(status)
        self.LineStyle.setEnabled(status)
        self.MarkerStyle.setEnabled(status)

        self.LineWidth.setEnabled(status and self._settings['linestyle'] != '')
        self.MarkerSize.setEnabled(status and self._settings['marker'] != '')

        self.clicked.emit()

    def repr(self, time=None):
        """
        Create scriptable version of the settings.

        Assume:
           'axes0', 'axes1' the left and right axes, respectively
           'x', 'y' hold the x/y data to be plotted.
        """

        settings = self.settings()
        ax = ['axes0', 'axes1'][settings.pop('axis')]

        s = []
        for key, value in settings.items():
                if key == 'color':
                    value = [round(v, 3) for v in value]
                s += [key + '=' + repr(value)]

        if time:
            output = ['y = data({}, time={})'.format(repr(settings['label']), repr(time))]
        else:
            output = ['y = data({})'.format(repr(settings['label']))]
        output += ['{}.plot(x, y, {})'.format(ax, ', '.join(s))]
        return output, []

    def _setupCheckBox(self, qobject):
        """
        Setup method for the CheckBox.
        """
        qobject.clicked.connect(self._callbackCheckBox)
        qobject.setFixedWidth(100)
        qobject.setToolTip(qobject.text())

        metrics = qobject.fontMetrics()
        elidedText = metrics.elidedText(qobject.text(), QtCore.Qt.ElideRight, qobject.width()-20);
        qobject.setText(elidedText);

    def _callbackCheckBox(self, value):
        """
        Callback for the checkbox, emits the clicked signal.
        """
        self.update()

    def _setupPlotAxis(self, qobject):
        """
        Setup method for plot type selection.
        """
        qobject.addItem('left')
        qobject.addItem('right')
        qobject.currentIndexChanged.connect(self._callbackPlotAxis)


    @QtCore.pyqtSlot()
    def _callbackPlotAxis(self):
        """
        Callback for changing the plot type.
        """
        self._settings['axis'] = self.PlotAxis.currentIndex()
        self.update()

    def _setupColorButton(self, qobject):
        """
        Setup method for the Artist color.
        """
        qobject.setStyleSheet('border:none;')
        qobject.setMaximumWidth(qobject.height())
        qobject.setAutoFillBackground(False)
        color = self._settings['color']
        c = QtGui.QColor(int(color[0]*255), int(color[1]*255), int(color[2]*255))
        qobject.setStyleSheet('border:none; background:rgb' + str(c.getRgb()))
        qobject.clicked.connect(self._callbackColorButton)


    def _callbackColorButton(self, test=None):
        """
        Callback for the color dialog.
        """
        dialog = QtWidgets.QColorDialog()
        c = dialog.getColor().getRgb()
        self.ColorButton.setStyleSheet('border:none; background:rgb' + str(c))
        c = [c[0]/255., c[1]/255., c[2]/255.]
        self._settings['color'] = c
        self.update()


    def _setupLineStyle(self, qobject):
        """
        Setup method for the line style.
        """
        for i in range(len(self.line_styles)):
            style = self.line_styles[i]
            qobject.addItem(style)
            if style == self._settings['linestyle']:
                qobject.setCurrentIndex(i)
        font = QtGui.QFont()
        font.setPointSize(11)
        qobject.setFont(font)
        qobject.currentIndexChanged.connect(self._callbackLineStyle)


    def _callbackLineStyle(self, index):
        """
        Callback for the line style widget.
        """
        style = self.LineStyle.currentText()
        self._settings['linestyle'] = style
        self.update()


    def _setupLineWidth(self, qobject):
        """
        Setup method for the line width.
        """
        font = QtGui.QFont()
        font.setPointSize(11)
        qobject.setFont(font)
        qobject.setDecimals(1)
        qobject.setValue(1.0)
        qobject.valueChanged.connect(self._callbackLineWidth)


    def _callbackLineWidth(self, value):
        """
        Callback for the line width widget.
        """
        self._settings['linewidth'] = value
        self.update()


    def _setupMarkerStyle(self, qobject):
        """
        Setup method for the marker style widget.
        """
        for i in range(len(self.marker_styles)):
            marker = self.marker_styles[i]
            qobject.addItem(marker)
            if marker == self._settings['marker']:
                qobject.setCurrentIndex(i)

        font = QtGui.QFont()
        font.setPointSize(11)
        qobject.setFont(font)
        qobject.currentIndexChanged.connect(self._callbackMarkerStyle)


    def _callbackMarkerStyle(self, index):
        """
        Callback for marker style selection.
        """
        style = self.MarkerStyle.currentText()

        # Update the size to the default
        if 'markersize' not in self._settings:
            size = self._artist.get_markersize()
            self.MarkerSize.setValue(size)

        self._settings['marker'] = style
        self.update()


    def _setupMarkerSize(self, qobject):
        """
        Setup method for the marker size.
        """
        font = QtGui.QFont()
        font.setPointSize(11)
        qobject.setFont(font)
        qobject.setDecimals(1)
        qobject.setValue(1.0)
        qobject.valueChanged.connect(self._callbackMarkerSize)
        qobject.setEnabled(False)


    def _callbackMarkerSize(self, value):
        """
        Callback for the marker size widget.
        """
        self._settings['markersize'] = value
        self.update()

def main(*args):
    """
    Create a LineSettingsWidget for testing.
    """
    from ..PostprocessorViewer import PostprocessorViewer
    from .FigurePlugin import FigurePlugin

    import matplotlib
    matplotlib.rcParams["figure.figsize"] = (3.75, 3.75)
    matplotlib.rcParams["figure.dpi"] = (100)

    # Load the viewer
    widget = PostprocessorViewer(plugins=[FigurePlugin])
    widget.onSetFilenames(['empty_file'])
    layout = widget.currentWidget().LeftLayout
    window =  widget.currentWidget().FigurePlugin
    window.setFixedSize(QtCore.QSize(375, 375))

    # Test the line setting widget
    toggle = LineSettingsWidget(*args)
    layout.addWidget(toggle)
    widget.show()
    return widget, toggle, window


if __name__ == '__main__':
    app = QtWidgets.QApplication(sys.argv)
    widget, control, window = main('exp')

    def graph():
        value = control.isValid()
        settings = control.settings()
        ax = window.axes()[settings.pop('axis')]
        window.clear()
        if value:
            ax.plot([0,1,2,4], [0,1,4,16], **settings)
        window.draw()

    control.clicked.connect(graph)
    sys.exit(app.exec_())
