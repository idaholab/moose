from PyQt5 import QtCore, QtWidgets
import chigger
import peacock
from peacock.ExodusViewer.plugins.ExodusPlugin import ExodusPlugin
from MeshBlockSelectorWidget import MeshBlockSelectorWidget

class BlockHighlighterPlugin(peacock.base.PeacockCollapsibleWidget, ExodusPlugin):
    """
    Widget for controlling the visible blocks/nodesets/sidesets of the mesh.
    Mirrored off of peaocock.Exodus.plugins.BlockPlugin
    """

    #: pyqtSignal: Emitted when window needs to change
    windowRequiresUpdate = QtCore.pyqtSignal()
    highlight = QtCore.pyqtSignal(object, object, object)

    def __init__(self, collapsible_layout=QtWidgets.QHBoxLayout, **kwargs):
        peacock.base.PeacockCollapsibleWidget.__init__(self, collapsible_layout=collapsible_layout)
        ExodusPlugin.__init__(self, **kwargs)

        self.setTitle('Highlight')
        self.setEnabled(False)

        self.MainLayout = self.collapsibleLayout()

        # Block, nodeset, and sideset selector widgets
        self.BlockSelector = MeshBlockSelectorWidget(chigger.exodus.ExodusReader.BLOCK, 'Blocks:')
        self.SidesetSelector = MeshBlockSelectorWidget(chigger.exodus.ExodusReader.SIDESET, 'Boundaries:')
        self.NodesetSelector = MeshBlockSelectorWidget(chigger.exodus.ExodusReader.NODESET, 'Nodesets:')

        self.MainLayout.addWidget(self.BlockSelector)
        self.MainLayout.addWidget(self.SidesetSelector)
        self.MainLayout.addWidget(self.NodesetSelector)

        self.BlockSelector.selectionChanged.connect(self._callbackSelector)
        self.SidesetSelector.selectionChanged.connect(self._callbackSelector)
        self.NodesetSelector.selectionChanged.connect(self._callbackSelector)

        self.setup()

    def onWindowCreated(self, *args):
        """
        Initializes the selector widgets for the supplied reader/results.
        """
        super(BlockHighlighterPlugin, self).onWindowCreated(*args)
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

    def _callbackSelector(self):
        """
        Updates the visible block/nodesets/sidesets based on the selector widget settings.
        """
        block = self.BlockSelector.getBlocks()
        sideset = self.SidesetSelector.getBlocks()
        nodeset = self.NodesetSelector.getBlocks()
        self.windowRequiresUpdate.emit()
        self.highlight.emit(block, sideset, nodeset)

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
