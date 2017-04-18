from peacock.ExodusViewer.plugins.BlockPlugin import BlockPlugin
from PyQt5 import QtCore, QtWidgets

class BlockHighlighterPlugin(BlockPlugin):
    """
    Highlights selected blocks/sidesets/nodesets.
    """
    highlight = QtCore.pyqtSignal(object, object, object)

    def __init__(self, collapsible_layout=QtWidgets.QHBoxLayout, **kwargs):
        super(BlockHighlighterPlugin, self).__init__(collapsible_layout=collapsible_layout, **kwargs)
        self.setTitle('Highlight')
        self.BlockSelector.setAllChecked(False)

    def _callbackSelector(self):
        """
        Override callback to send the highlight signal instead of the options changed signals.
        """
        block = self.BlockSelector.getBlocks()
        sideset = self.SidesetSelector.getBlocks()
        nodeset = self.NodesetSelector.getBlocks()
        self.highlight.emit(block, sideset, nodeset)
        self.windowRequiresUpdate.emit()
