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
from .ExodusPlugin import ExodusPlugin
from .BlockSelectorWidget import BlockSelectorWidget

class BlockPlugin(QtWidgets.QGroupBox, ExodusPlugin):
    """
    Widget for controlling the visible blocks/nodesets/sidesets of the result.
    """

    #: pyqtSignal: Emitted when window needs to change
    windowRequiresUpdate = QtCore.pyqtSignal()

    #: pyqtSignal: Emitted when the chigger reader options are changed
    readerOptionsChanged = QtCore.pyqtSignal(dict)

    #: pyqtSignal: Emitted when the chigger result options are changed
    resultOptionsChanged = QtCore.pyqtSignal(dict)

    def __init__(self, **kwargs):
        super(BlockPlugin, self).__init__()

        self.setSizePolicy(QtWidgets.QSizePolicy.Maximum, QtWidgets.QSizePolicy.Maximum)

        self.MainLayout = QtWidgets.QHBoxLayout(self)
        self.MainLayout.setSpacing(10)
        self.setEnabled(False)

        # Block, nodeset, and sideset selector widgets
        self.BlockSelector = BlockSelectorWidget(chigger.exodus.ExodusReader.BLOCK, title='Blocks')
        self.SidesetSelector = BlockSelectorWidget(chigger.exodus.ExodusReader.SIDESET, title='Boundaries', enabled=False)
        self.NodesetSelector = BlockSelectorWidget(chigger.exodus.ExodusReader.NODESET, title='Nodesets', enabled=False)

        self.MainLayout.addWidget(self.BlockSelector)
        self.MainLayout.addWidget(self.SidesetSelector)
        self.MainLayout.addWidget(self.NodesetSelector)

        self.setup()
        self._varinfo = None  # current variable names

    def onSetFilename(self, *args):
        super(BlockPlugin, self).onSetFilename(*args)
        self._loadPlugin()
        self.updateOptions()

    def onSetVariable(self, *args):
        """
        Loads the selected items when the variable changes.
        """
        super(BlockPlugin, self).onSetVariable(*args)
        self._loadPlugin()
        self.updateOptions()

    def onSetComponent(self, *args):
        """
        Loads the selected items when the variable component changes.
        """
        super(BlockPlugin, self).onSetComponent(*args)
        self._loadPlugin()
        self.updateOptions()

    def onResetWindow(self):
        """
        Remove variable information when the window is reset.
        """
        self._varinfo = None

    def onSetupResult(self, result):
        """
        Update the block selections when the result object is created (i.e., when the file changes).
        """
        reader = result[0].getExodusReader()
        self.BlockSelector.updateBlocks(reader)
        self.SidesetSelector.updateBlocks(reader)
        self.NodesetSelector.updateBlocks(reader)
        self._varinfo = reader.getVariableInformation([chigger.exodus.ExodusReader.NODAL])

        self._loadPlugin()
        self.updateOptions()

    def onUpdateWindow(self, window, reader, result):
        """
        Update the block list, if needed.
        """
        blocks = self._blocksChanged(reader, self.BlockSelector, chigger.exodus.ExodusReader.BLOCK)
        sideset = self._blocksChanged(reader, self.SidesetSelector, chigger.exodus.ExodusReader.SIDESET)
        nodeset = self._blocksChanged(reader, self.NodesetSelector, chigger.exodus.ExodusReader.NODESET)
        if any([blocks, sideset, nodeset]):
            self.onSetupResult(result)

    def updateReaderOptions(self):
        """
        Update the ExodusReader options.
        """
        options = self._getBlockOptions()
        self.readerOptionsChanged.emit(options)
        self.resultOptionsChanged.emit(options)

    def updateResultOptions(self):
        """
        Update the ExodusResult options.
        """
        options = self._getBlockOptions()
        self.resultOptionsChanged.emit(options)

    def updateOptions(self):
        """
        Updates the block, boundary, and nodeset settings for the reader/result objects.
        """
        if self._varinfo:
            options = self._getBlockOptions()
            self.readerOptionsChanged.emit(options)
            self.resultOptionsChanged.emit(options)

    def _getBlockOptions(self):
        """
        Helper to return the block/boundary/nodeset options for the reader and result objects.
        """
        options = dict()
        if self._variable in self._varinfo:
            self.BlockSelector.setEnabled(True)
            self.SidesetSelector.setEnabled(True)
            self.NodesetSelector.setEnabled(True)

            options['block'] = self.BlockSelector.getBlocks()
            options['boundary'] = self.SidesetSelector.getBlocks()
            options['nodeset'] = self.NodesetSelector.getBlocks()

        # Elemental
        else:
            options['block'] = self.BlockSelector.getBlocks()
            options['boundary'] = None
            options['nodeset'] = None
            self.BlockSelector.setEnabled(True)
            self.SidesetSelector.setEnabled(False)
            self.NodesetSelector.setEnabled(False)
        return options

    def _blocksChanged(self, reader, qobject, btype):
        """
        Check if current blocks on the widget are the same as exist on the reader.
        """
        blk_info = reader.getBlockInformation()[btype]
        blocks = [blk.name for blk in blk_info.values()]
        current = [qobject.StandardItemModel.item(i).data(QtCore.Qt.UserRole) for i in range(1, qobject.StandardItemModel.rowCount())]
        return set(blocks) != set(current)

    def _loadPlugin(self):
        """
        Helper for loading the state of the various selector widgets.
        """
        if self._varinfo is None:
            return

        self.BlockSelector.load(self.stateKey())
        if not self.BlockSelector.hasState(self.stateKey()):
            self.BlockSelector.StandardItemModel.item(0).setCheckState(QtCore.Qt.Checked)
            self.BlockSelector.itemsChanged.emit()

        self.SidesetSelector.load(self.stateKey())
        if not self.SidesetSelector.hasState(self.stateKey()):
            self.SidesetSelector.StandardItemModel.item(0).setCheckState(QtCore.Qt.Unchecked)
            self.SidesetSelector.itemsChanged.emit()

        self.NodesetSelector.load(self.stateKey())
        if not self.NodesetSelector.hasState(self.stateKey()):
            self.NodesetSelector.StandardItemModel.item(0).setCheckState(QtCore.Qt.Unchecked)
            self.NodesetSelector.itemsChanged.emit()

    def _setupBlockSelector(self, qobject):
        qobject.itemsChanged.connect(self._callbackBlockSelector)

    def _callbackBlockSelector(self):
        self.BlockSelector.store(self.stateKey())
        self.updateOptions()
        self.windowRequiresUpdate.emit()

    def _setupSidesetSelector(self, qobject):
        qobject.itemsChanged.connect(self._callbackSidesetSelector)

    def _callbackSidesetSelector(self):
        self.SidesetSelector.store(self.stateKey())
        self.updateOptions()
        self.windowRequiresUpdate.emit()

    def _setupNodesetSelector(self, qobject):
        qobject.itemsChanged.connect(self._callbackNodesetSelector)

    def _callbackNodesetSelector(self):
        self.NodesetSelector.store(self.stateKey())
        self.updateOptions()
        self.windowRequiresUpdate.emit()

def main(size=None):
    """
    Run the BlockPlugin all by its lonesome.
    """
    from peacock.ExodusViewer.ExodusPluginManager import ExodusPluginManager
    from .VTKWindowPlugin import VTKWindowPlugin
    from .FilePlugin import FilePlugin
    widget = ExodusPluginManager(plugins=[lambda: VTKWindowPlugin(size=size),
                                          FilePlugin,
                                          BlockPlugin])
    widget.show()

    return widget, widget.VTKWindowPlugin

if __name__ == '__main__':
    from peacock.utils import Testing
    app = QtWidgets.QApplication(sys.argv)
    filenames = Testing.get_chigger_input_list('mug_blocks_out.e', 'mesh_only.e', 'vector_out.e', 'displace.e')
    #filenames = Testing.get_chigger_input_list('none.e')
    widget, window = main(size=[600,600])
    widget.FilePlugin.onSetFilenames(filenames)
    sys.exit(app.exec_())
