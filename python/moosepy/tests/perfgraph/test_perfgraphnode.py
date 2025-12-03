# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Tests moosepy.perfgraph.perfgraphnode.TestPerfGraphNode."""

from unittest import TestCase

from moosepy.tests.perfgraph.common import (
    NODE_KWARGS,
    build_test_node,
    build_test_section,
)


class TestPerfGraphNode(TestCase):
    """Tests moosepy.perfgraph.perfgraphnode.TestPerfGraphNode."""

    def test_init(self):
        """Test __init__()."""
        section = build_test_section()
        parent = build_test_node()
        node = build_test_node(section=section, parent=parent)

        # Variables that get set directly from input
        self.assertEqual(node._id, NODE_KWARGS["id"])
        self.assertEqual(node._name, NODE_KWARGS["name"])
        self.assertEqual(node._self_time, NODE_KWARGS["time"])
        self.assertEqual(node._section, section)
        self.assertEqual(node._parent, parent)

        # Adds to the parent
        self.assertIn(node, parent.children)
        # Adds to the section
        self.assertIn(node, section.nodes)

    def test_init_no_parent(self):
        """Test __init__() without a parent."""
        section = build_test_section()
        build_test_node(section=section)

    def test_str(self):
        """Test __str__()."""
        node = build_test_node()
        self.assertEqual(str(node), f"PerfGraphNode {NODE_KWARGS['name']}")

    def test_id(self):
        """Test property id."""
        self.assertEqual(build_test_node().id, NODE_KWARGS["id"])

    def test_name(self):
        """Test property name."""
        self.assertEqual(build_test_node().name, NODE_KWARGS["name"])

    def test_self_time(self):
        """Test property self_time."""
        self.assertEqual(build_test_node().self_time, NODE_KWARGS["time"])

    def test_num_calls(self):
        """Test property num_calls."""
        self.assertEqual(build_test_node().num_calls, NODE_KWARGS["num_calls"])

    def test_section(self):
        """Test property section."""
        section = build_test_section()
        node = build_test_node(section=section)
        self.assertEqual(node.section, section)

    def test_parent(self):
        """Test property parent."""
        parent = build_test_node()
        node = build_test_node(parent=parent)
        self.assertEqual(node.parent, parent)

    def test_children(self):
        """Test property children."""
        child = build_test_node()
        node = build_test_node()
        self.assertEqual(len(node.children), 0)
        child._parent = node
        node._add_child(child)
        self.assertEqual(len(node.children), 1)
        self.assertEqual(id(child), id(node.children[0]))

    def test_level(self):
        """Test property level."""
        level = 100
        section = build_test_section(level=100)
        node = build_test_node(section=section)
        self.assertEqual(node.level, level)

    def test_children_time(self):
        """Test property children_time."""
        node = build_test_node()
        self.assertEqual(node.children_time, 0.0)

        times = [1.0, 2.0]
        [build_test_node(time=v, parent=node) for v in times]
        self.assertEqual(node.children_time, sum(times))

    def test_total_time(self):
        """Test property total_time."""
        self_time = 1.1
        node = build_test_node(time=1.1)
        self.assertEqual(node.total_time, self_time)

        child_time = 1.2
        build_test_node(time=1.2, parent=node)
        self.assertEqual(node.total_time, self_time + child_time)

    def test_self_percent_time(self):
        """Test property self_percent_time."""
        self_time = 1.2
        root_time = 0.1

        root_node = build_test_node(time=root_time)
        node = build_test_node(time=self_time, parent=root_node)
        self.assertAlmostEqual(
            node.self_percent_time, self_time * 100 / (self_time + root_time)
        )

    def test_children_percent_time(self):
        """Test property children_percent_time."""
        self_time = 1.1
        child_time = 1.2
        root_time = 1.3

        root_node = build_test_node(time=root_time)
        node = build_test_node(time=self_time, parent=root_node)
        build_test_node(time=child_time, parent=node)
        self.assertAlmostEqual(
            node.children_percent_time,
            child_time * 100 / (root_time + self_time + child_time),
        )

    def test_total_percent_time(self):
        """Test property total_percent_time."""
        self_time = 1.1
        root_time = 1.2
        child_time = 1.3

        root_node = build_test_node(time=root_time)
        node = build_test_node(time=self_time, parent=root_node)
        build_test_node(time=child_time, parent=node)
        self.assertAlmostEqual(
            node.total_percent_time,
            (self_time + child_time) * 100 / (root_time + self_time + child_time),
        )

    def test_root_node(self):
        """Test property root_node."""
        root = build_test_node()
        self.assertEqual(root.root_node, root)

        node = build_test_node(parent=root)
        self.assertEqual(node.root_node, root)

        child = build_test_node(parent=node)
        self.assertEqual(child.root_node, root)

    def test_path(self):
        """Test property path."""
        foo = build_test_node(name="foo")
        self.assertEqual(foo.path, ["foo"])

        bar = build_test_node(name="bar", parent=foo)
        self.assertEqual(bar.path, ["foo", "bar"])

        baz = build_test_node(name="baz", parent=bar)
        self.assertEqual(baz.path, ["foo", "bar", "baz"])

    def test_add_child(self):
        """Test _add_child()."""
        root = build_test_node()
        node = build_test_node()
        node._parent = root
        root._add_child(node)
        self.assertEqual(root.children, [node])

    def test_query_child(self):
        """Test query_child()."""
        root = build_test_node()

        self.assertIsNone(root.query_child("foo"))

        child1 = build_test_node(name="child1", parent=root)
        child2 = build_test_node(name="child2", parent=root)
        self.assertEqual(id(root.query_child("child1")), id(child1))
        self.assertEqual(id(root.query_child("child2")), id(child2))

    def test_has_child(self):
        """Test has_child()."""
        root = build_test_node()

        self.assertFalse(root.has_child("foo"))

        build_test_node(name="foo", parent=root)
        self.assertTrue(root.has_child("foo"))

    def test_get_child(self):
        """Test get_child()."""
        root = build_test_node()

        with self.assertRaisesRegex(KeyError, "Child with name 'foo' does not exist"):
            root.get_child("foo")

        child = build_test_node(name="foo", parent=root)
        self.assertEqual(id(root.get_child("foo")), id(child))
