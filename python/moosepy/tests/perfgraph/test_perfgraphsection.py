# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Tests moosepy.perfgraph.perfgraphsection.TestPerfGraphSection."""

from unittest import TestCase

from moosepy.perfgraph.perfgraphsection import PerfGraphSection
from moosepy.tests.perfgraph.common import DummyNode


class DummyPerfGraphSection(PerfGraphSection):
    """Dummy PerfGraphSection for testing."""

    def __init__(self, nodes=None):
        """Fake init; sets the nodes and a dummy name and level."""
        super().__init__("dummy_name", 1)
        if nodes is not None:
            self._nodes = nodes


class TestPerfGraphSection(TestCase):
    """Tests moosepy.perfgraph.perfgraphsection.TestPerfGraphSection."""

    def test_init(self):
        """Test __init__()."""
        name = "dummy_name"
        level = 1
        section = PerfGraphSection(name, level)
        self.assertEqual(section._name, name)
        self.assertEqual(section._level, level)

    def test_str(self):
        """Test __str__()."""
        name = "dummy_name"
        section = PerfGraphSection(name, 0)
        self.assertEqual(str(section), f'PerfGraphSection "{name}"')

    def test_nodes(self):
        """Test property nodes."""
        nodes = [DummyNode()]
        section = DummyPerfGraphSection(nodes)
        self.assertEqual(section.nodes, nodes)
