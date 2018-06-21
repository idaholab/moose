#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

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

        self.BlockSelector.selectionChanged.connect(self.setBlock)
        self.SidesetSelector.selectionChanged.connect(self.setSideset)
        self.NodesetSelector.selectionChanged.connect(self.setNodeset)

        self.setup()

    def onWindowCreated(self, *args):
        """
        Initializes the selector widgets for the supplied reader/results.
        """
        super(BlockHighlighterPlugin, self).onWindowCreated(*args)
        self.BlockSelector.updateBlocks(self._reader, True)
        self.SidesetSelector.updateBlocks(self._reader, True)
        self.NodesetSelector.updateBlocks(self._reader, True)
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

    def setBlock(self):
        """
        Highlights a block and resets nodesets/sidesets
        """
        block = self.BlockSelector.getBlocks()
        self.SidesetSelector.reset()
        self.NodesetSelector.reset()
        self.highlight.emit(block, None, None)

    def setSideset(self):
        """
        Highlights a sideset and resets nodesets/blocks
        """
        sideset = self.SidesetSelector.getBlocks()
        self.BlockSelector.reset()
        self.NodesetSelector.reset()
        self.highlight.emit(None, sideset, None)

    def setNodeset(self):
        """
        Highlights a nodeset and resets sidesets/blocks
        """
        nodeset = self.NodesetSelector.getBlocks()
        self.BlockSelector.reset()
        self.SidesetSelector.reset()
        self.highlight.emit(None, None, nodeset)

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
