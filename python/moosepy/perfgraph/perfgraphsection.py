# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Implements PerfGraphSection, which represents a section in the MOOSE PerfGraph."""

from typing import TYPE_CHECKING

from moosepy.perfgraph.perfgraphobject import PerfGraphObject

if TYPE_CHECKING:
    from moosepy.perfgraph.perfgraphnode import PerfGraphNode


class PerfGraphSection(PerfGraphObject):
    """
    A section in the graph for the PerfGraph.

    Should be constructed internally in the PerfGraph object.
    """

    def __init__(self, name: str, level: int):
        """
        Initialize state.

        Arguments:
        ---------
        name : str
            The name of the section.
        level : int
            The level for this section.

        """
        super().__init__(name, level)

    def __str__(self):
        """Human-readable name for this section."""
        return 'PerfGraphSection "' + self.name + '"'

    @property
    def nodes(self) -> list["PerfGraphNode"]:
        """Get the nodes in this section."""
        return self._nodes
