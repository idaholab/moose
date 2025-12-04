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

from mock import patch
from moosepy.perfgraph.perfgraphnode import PerfGraphNode
from moosepy.perfgraph.perfgraphsection import PerfGraphSection
from moosepy.tests.perfgraph.common import (
    SECTION_KWARGS,
    build_test_node,
    build_test_section,
)


class TestPerfGraphSection(TestCase):
    """Tests moosepy.perfgraph.perfgraphsection.TestPerfGraphSection."""

    def test_init(self):
        """Test __init__()."""
        section = build_test_section()
        self.assertEqual(section._name, SECTION_KWARGS["name"])
        self.assertEqual(section._level, SECTION_KWARGS["level"])

    def test_str(self):
        """Test __str__()."""
        name = "dummy_name"
        section = build_test_section(name=name)
        self.assertEqual(str(section), f'PerfGraphSection "{name}"')

    def test_name(self):
        """Test property name."""
        name = "foo"
        section = build_test_section(name=name)
        self.assertEqual(section.name, name)

    def test_level(self):
        """Test property level."""
        level = 2
        section = build_test_section(level=2)
        self.assertEqual(section.level, level)

    def test_nodes(self):
        """Test property nodes."""
        nodes = [build_test_node()]
        section = build_test_section(nodes)
        self.assertEqual(section.nodes, nodes)

    def test_num_calls(self):
        """Test property num_calls."""
        num_calls = [5, 6]
        nodes = [build_test_node(num_calls=v) for v in num_calls]
        section = build_test_section(nodes)
        self.assertEqual(section.num_calls, sum(num_calls))

    def test_self_time(self):
        """Test property self_time."""
        self_time = [1.23, 4.56]
        nodes = [build_test_node(time=v) for v in self_time]
        section = build_test_section(nodes)
        self.assertEqual(section.self_time, sum(self_time))

    def run_mocked_property_test(self, method_name: str):
        """Test mocking node properties and calling the equivalent section property."""
        values = [1.1, 2.2, 3.3]
        nodes = [build_test_node() for _ in values]
        section = build_test_section(nodes)
        with patch.object(PerfGraphNode, method_name):
            for i, time in enumerate(values):
                setattr(nodes[i], method_name, time)
            self.assertEqual(getattr(section, method_name), sum(values))

    def test_children_time(self):
        """Test property children_time."""
        self.run_mocked_property_test("children_time")

    def test_total_time(self):
        """Test property total_time."""
        self_time = 0.5
        children_time = 1.5

        obj = build_test_section()
        with (
            patch.object(PerfGraphSection, "self_time"),
            patch.object(PerfGraphSection, "children_time"),
        ):
            obj.self_time = self_time
            obj.children_time = children_time
            self.assertEqual(obj.total_time, self_time + children_time)

    def test_self_percent_time(self):
        """Test property self_percent_time."""
        self.run_mocked_property_test("self_percent_time")

    def test_children_percent_time(self):
        """Test property children_percent_time."""
        self.run_mocked_property_test("children_percent_time")

    def test_total_percent_time(self):
        """Test property total_percent_time."""
        self.run_mocked_property_test("total_percent_time")

    def test_add_node(self):
        """Test _add_node()."""
        section = build_test_section()
        node = build_test_node()
        section._add_node(node)
        self.assertEqual(len(section.nodes), 1)
        self.assertEqual(id(section.nodes[0]), id(node))

    def test_query_node(self):
        """Test query_node()."""
        root = build_test_node()
        section = build_test_section()
        child = build_test_node(parent=root, section=section)
        self.assertIsNone(section.query_node(["foo"]))
        self.assertEqual(id(child), id(section.query_node(child.path)))

    def test_has_node(self):
        """Test has_node()."""
        root = build_test_node()
        section = build_test_section()
        child = build_test_node(parent=root, section=section)
        self.assertFalse(section.has_node(["foo"]))
        self.assertTrue(section.has_node(child.path))

    def test_get_node(self):
        """Test get_node()."""
        root = build_test_node()
        section = build_test_section()
        child = build_test_node(parent=root, section=section)
        self.assertEqual(id(child), id(section.get_node(child.path)))
        with self.assertRaisesRegex(
            KeyError, r"Node with path \['foo'\] does not exist"
        ):
            section.get_node(["foo"])
