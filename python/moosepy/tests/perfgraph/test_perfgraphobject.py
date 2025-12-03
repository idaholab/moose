# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Tests moosepy.perfgraph.perfgraphobject.PerfGraphObject."""

from unittest import TestCase

from mock import patch
from moosepy.perfgraph.perfgraphobject import PerfGraphObject
from moosepy.tests.perfgraph.common import DummyNode


class DummyPerfGraphObject(PerfGraphObject):
    """Dummy PerfGraphObject for testing."""

    def __init__(self, nodes=None):
        """Fake init; sets the nodes and a dummy name and level."""
        super().__init__("dummy_name", 1)
        if nodes is not None:
            self._nodes = nodes


class TestPerfGraphObject(TestCase):
    """Tests moosepy.perfgraph.perfgraphobject.PerfGraphObject."""

    def test_init(self):
        """Test __init__()."""
        name = "foo"
        level = 2
        obj = PerfGraphObject(name, level)
        self.assertEqual(obj._name, name)
        self.assertEqual(obj._level, level)

    def test_name(self):
        """Test property name."""
        name = "foo"
        obj = PerfGraphObject(name, 0)
        self.assertEqual(obj.name, name)

    def test_level(self):
        """Test property level."""
        level = 1
        obj = PerfGraphObject("unused", level)
        self.assertEqual(obj.level, level)

    def test_sum_all_nodes(self):
        """Test _sum_all_nodes()."""
        calls = [1, 2]
        nodes = [DummyNode(value=v) for v in calls]
        obj = DummyPerfGraphObject(nodes)
        self.assertEqual(obj._sum_all_nodes(lambda n: n.value), sum(calls))

    def test_num_calls(self):
        """Test property num_calls."""
        calls = [1, 2]
        nodes = [DummyNode(_data={"num_calls": v}) for v in calls]
        obj = DummyPerfGraphObject(nodes)
        self.assertEqual(obj.num_calls, sum(calls))

    def test_self_time(self):
        """Test property self_time."""
        nodes = [DummyNode(), DummyNode()]
        times = [1.0, 2.0]

        obj = DummyPerfGraphObject(nodes)
        with patch.object(DummyNode, "time"):
            for i in range(len(nodes)):
                nodes[i].time = times[i]
            self.assertEqual(obj.self_time, sum(times))

    def test_children_time(self):
        """Test property children_time."""
        node00 = DummyNode()
        node0 = DummyNode(_children={"node00": node00})

        node10 = DummyNode()
        node1 = DummyNode(_children={"node10": node10})

        obj = DummyPerfGraphObject([node0, node1])
        times = [0.5, 1.0]

        with patch.object(DummyNode, "total_time"):
            node00.total_time = times[0]
            node10.total_time = times[1]
            self.assertEqual(obj.children_time, sum(times))

    def test_total_time(self):
        """Test property total_time."""
        self_time = 0.5
        children_time = 1.5

        obj = DummyPerfGraphObject()
        with (
            patch.object(PerfGraphObject, "self_time"),
            patch.object(PerfGraphObject, "children_time"),
        ):
            obj.self_time = self_time
            obj.children_time = children_time
            self.assertEqual(obj.total_time, self_time + children_time)

    def test_percent_time(self):
        """Test property percent_time."""
        total_time = 2.0
        root_total_time = 4.5

        obj = DummyPerfGraphObject()
        root = DummyNode()
        with (
            patch.object(PerfGraphObject, "total_time"),
            patch.object(PerfGraphObject, "root_node"),
            patch.object(DummyNode, "total_time"),
        ):
            obj.total_time = total_time
            obj.root_node = root
            root.total_time = root_total_time
            self.assertEqual(obj.percent_time, total_time * 100 / root_total_time)

    def test_root_node(self):
        """Test property root_node."""
        root = DummyNode(_parent=None)
        node1 = DummyNode(_parent=root)
        node2 = DummyNode(_parent=node1)

        self.assertEqual(node2.root_node, root)
        self.assertEqual(node1.root_node, root)
        self.assertEqual(root.root_node, root)
