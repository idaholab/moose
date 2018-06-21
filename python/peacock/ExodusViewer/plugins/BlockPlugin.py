#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from PyQt5 import QtCore, QtWidgets
import sys
import chigger
import peacock
from ExodusPlugin import ExodusPlugin
from BlockSelectorWidget import BlockSelectorWidget

class BlockPlugin(peacock.base.PeacockCollapsibleWidget, ExodusPlugin):
    """
    Widget for controlling the visible blocks/nodesets/sidesets of the result.
    """

    #: pyqtSignal: Emitted when window needs to change
    windowRequiresUpdate = QtCore.pyqtSignal()

    #: pyqtSignal: Emitted when the chigger objects options are changed
    resultOptionsChanged = QtCore.pyqtSignal(dict)
    readerOptionsChanged = QtCore.pyqtSignal(dict)

    def __init__(self, collapsible_layout=QtWidgets.QHBoxLayout, **kwargs):
        peacock.base.PeacockCollapsibleWidget.__init__(self, collapsible_layout=collapsible_layout)
        ExodusPlugin.__init__(self, **kwargs)

        # Current variable (used for caching settings
        self._contour = False
        self.setSizePolicy(QtWidgets.QSizePolicy.Maximum, QtWidgets.QSizePolicy.Maximum)

        # Setup this widget
        self.setTitle('Block Selection')
        self.setEnabled(False)

        # MainLayout
        self.MainLayout = self.collapsibleLayout()

        # Block, nodeset, and sideset selector widgets
        self.BlockSelector = BlockSelectorWidget(chigger.exodus.ExodusReader.BLOCK, title='Blocks:')
        self.SidesetSelector = BlockSelectorWidget(chigger.exodus.ExodusReader.SIDESET, title='Boundaries:', enabled=False)
        self.NodesetSelector = BlockSelectorWidget(chigger.exodus.ExodusReader.NODESET, title='Nodesets:', enabled=False)

        self.MainLayout.addWidget(self.BlockSelector)
        self.MainLayout.addWidget(self.SidesetSelector)
        self.MainLayout.addWidget(self.NodesetSelector)
        self.MainLayout.addStretch(1)

        self.BlockSelector.clicked.connect(self._callbackSelector)
        self.SidesetSelector.clicked.connect(self._callbackSelector)
        self.NodesetSelector.clicked.connect(self._callbackSelector)
        self._selectors = [self.BlockSelector, self.SidesetSelector, self.NodesetSelector]

        self.setup()

    def onVariableChanged(self, *args):
        """
        When a variable changes, load the state of the clip.
        """
        super(BlockPlugin, self).onVariableChanged(*args)
        self.load(self.stateKey(self._variable), 'Variable')
        if self._result:
            self.__updateVariableState()
            self._callbackSelector()

    def onContourClicked(self, state):
        """
        Slot for setting the state of the contour checkbox.

        Args:
            state[bool]: When True the contour results are generated rather than regular volume results.
        """
        self._contour = state
        self.SidesetSelector.setEnabled(not state)
        self.NodesetSelector.setEnabled(not state)
        self.load(self.stateKey(state), 'Contour')
        if self._result:
            self._callbackSelector()

    def onWindowCreated(self, *args):
        """
        Initializes the selector widgets for the supplied reader/results.
        """
        super(BlockPlugin, self).onWindowCreated(*args)
        self.BlockSelector.updateBlocks(self._reader)
        self.SidesetSelector.updateBlocks(self._reader)
        self.NodesetSelector.updateBlocks(self._reader)
        self.__updateVariableState()

    def onWindowUpdated(self):
        """
        Update boundary/nodeset visibility when window is updated.
        """
        if self._reader:
            self.blockSignals(True)
            self.BlockSelector.updateBlocks(self._reader)
            self.SidesetSelector.updateBlocks(self._reader)
            self.NodesetSelector.updateBlocks(self._reader)
            self.blockSignals(False)
            self.__updateVariableState()

            # Make the lists a uniform height
            h = max(x.ListWidget.maximumHeight() for x in self._selectors)
            for x in self._selectors:
                x.ListWidget.setMaximumHeight(h)


    def _callbackSelector(self):
        """
        Updates the visible block/nodesets/sidesets based on the selector widget settings.
        """

        self.store(self.stateKey(self._contour), 'Contour')
        self.store(self.stateKey(self._variable), 'Variable')

        options = dict()
        options['block'] = self.BlockSelector.getBlocks()
        options['boundary'] = self.SidesetSelector.getBlocks()
        options['nodeset'] = self.NodesetSelector.getBlocks()
        self.readerOptionsChanged.emit(options)
        self.resultOptionsChanged.emit(options)
        self.windowRequiresUpdate.emit()

    def __updateVariableState(self):
        """
        Enable/disable the nodeset/sidest selection based on variable type.
        """
        varinfo = self._result[0].getCurrentVariableInformation()
        if varinfo:
            if varinfo.object_type == chigger.exodus.ExodusReader.ELEMENTAL:
                self.SidesetSelector.setEnabled(False)
                self.NodesetSelector.setEnabled(False)
            else:
                self.SidesetSelector.setEnabled(True)
                self.NodesetSelector.setEnabled(True)

def main(size=None):
    """
    Run the BlockPlugin all by its lonesome.
    """
    from peacock.ExodusViewer.ExodusPluginManager import ExodusPluginManager
    from peacock.ExodusViewer.plugins.VTKWindowPlugin import VTKWindowPlugin

    widget = ExodusPluginManager(plugins=[lambda: VTKWindowPlugin(size=size, layout='WindowLayout'),
                                          lambda: BlockPlugin(layout='WindowLayout')])
    widget.show()

    return widget, widget.VTKWindowPlugin

if __name__ == '__main__':
    from peacock.utils import Testing
    app = QtWidgets.QApplication(sys.argv)
    filenames = Testing.get_chigger_input_list('mug_blocks_out.e', 'vector_out.e', 'displace.e')
    widget, window = main()
    window.onFileChanged(filenames[0])
    window.onWindowRequiresUpdate()
    sys.exit(app.exec_())
