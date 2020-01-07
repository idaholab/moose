#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import peacock
from PyQt5 import QtWidgets
from peacock.utils import WidgetUtils
import mooseutils
from .PostprocessorPlugin import PostprocessorPlugin
from .PostprocessorTableWidget import PostprocessorTableWidget

class OutputPlugin(peacock.base.OutputWidgetBase, PostprocessorPlugin):
    """
    Plugin responsible for triggering the creation of png/pdf/py and live script viewing.
    """

    def __init__(self):
        super(OutputPlugin, self).__init__()

        # Adds the live table view
        self.LiveTableButton = QtWidgets.QPushButton()
        self.LiveTable = PostprocessorTableWidget()

        self.MainLayout.addWidget(self.LiveTableButton)
        self.MainLayout.addStretch()
        self.setup()

    def onSetData(self, data):
        """
        This is called when the data is changed.
        """
        self.LiveTable.initialize(data)

    def repr(self):
        """
        Return matplotlib scripting information.
        """
        output = ['', '# Show figure and write pdf', 'plt.show()', 'figure.savefig("output.pdf")']
        return output, []

    def onAxesModified(self):
        """
        Slot called when the window is changed.
        """
        self.updateLiveScriptText()

    def onTimeChanged(self, *args):
        """
        Called when time is changed.
        """
        if self.LiveTable.isVisible():
            self.LiveTable.onTimeChanged(*args)


    def onDataChanged(self):
        """
        Update the table.
        """
        if self.LiveTable.isVisible():
            self.LiveTable.onDataChanged()

    def _setupLiveTableButton(self, qobject):
        """
        Setup method for python script output button.
        """
        qobject.clicked.connect(self._callbackLiveTableButton)
        qobject.setIcon(WidgetUtils.createIcon('table.svg'))
        qobject.setIconSize(self._icon_size)
        qobject.setFixedSize(qobject.iconSize())
        qobject.setToolTip("Show a table of the CSV data for each open file.")
        qobject.setStyleSheet("QPushButton {border:none}")

    def _callbackLiveTableButton(self):
        """
        Callback for the live table toggle.
        """
        self.LiveTable.show()
        self.LiveTable.onDataChanged()


def main(filenames):

    from ..PostprocessorViewer import PostprocessorViewer
    from .PostprocessorSelectPlugin import PostprocessorSelectPlugin
    from .MediaControlPlugin import MediaControlPlugin

    widget = PostprocessorViewer(mooseutils.VectorPostprocessorReader, timeout=None, plugins=[OutputPlugin, PostprocessorSelectPlugin, MediaControlPlugin])
    widget.onSetFilenames(filenames)
    control = widget.currentWidget().OutputPlugin
    widget.show()

    return control, widget

if __name__ == '__main__':
    import sys
    app = QtWidgets.QApplication(sys.argv)
    #control, widget = main(['../../../tests/input/white_elephant_jan_2016.csv', '../../../tests/input/white_elephant_jan_2016.csv'])
    control, widget = main(['../../../tests/input/vpp_*.csv', '../../../tests/input/vpp2_*.csv'])

    sys.exit(app.exec_())
