# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Tests moosepy.perfgraph.perfgraph.TestPerfGraph."""

import json
import os
import subprocess
from copy import deepcopy
from tempfile import TemporaryDirectory
from typing import Optional
from unittest import TestCase

import pytest
from moosepy.perfgraph.perfgraph import PerfGraph
from moosepy.perfgraph.perfgraphnode import PerfGraphNode
from moosepy.perfgraph.perfgraphsection import PerfGraphSection
from moosepy.tests.perfgraph.common import (
    build_node_data,
    build_test_node,
    build_test_section,
)

NO_NODE_DATA = {"root": build_node_data()}


def build_perf_graph() -> PerfGraph:
    """Build a PerfGraph with dummpy data for testing."""
    return PerfGraph(deepcopy(NO_NODE_DATA))


class TestPerfGraph(TestCase):
    """Tests moosepy.perfgraph.perfgraph.TestPerfGraph."""

    @pytest.fixture(autouse=True)
    def inject_fixtures(self, moose_exe):
        """Inject pytest fixtures."""
        # Get the found moose executable during init, if any
        self.moose_exe: Optional[str] = moose_exe

    def test_init(self):
        """Test __init__()."""
        pg = build_perf_graph()
        self.assertIsInstance(pg._nodes, dict)
        self.assertIsInstance(pg._sections, dict)
        self.assertIsInstance(pg._root_node, PerfGraphNode)

    def test_parse_node_data(self):
        """Test _parse_node_data()."""

        def check_common(data, node_data, level):
            self.assertEqual(
                node_data,
                {
                    "time": data["time"],
                    "num_calls": data["num_calls"],
                    "memory": data["memory"],
                },
            )
            self.assertIsInstance(level, int)
            self.assertEqual(level, data["level"])
            self.assertIsInstance(children_data, list)

        root_args = {"time": 1.234, "num_calls": 100, "memory": 55.0, "level": 1}
        child_level = 2
        child_args = {
            "time": 5.678,
            "num_calls": 2,
            "memory": 100.2,
            "level": child_level,
        }

        # Just the root node, no children
        data = build_node_data(**root_args)
        node_data, level, children_data = PerfGraph._parse_node_data(deepcopy(data))
        check_common(data, node_data, level)
        self.assertIsInstance(children_data, list)
        self.assertFalse(children_data)

        # Single child
        data = {**build_node_data(), "child": build_node_data(**child_args)}
        node_data, level, children_data = PerfGraph._parse_node_data(deepcopy(data))
        check_common(data, node_data, level)
        self.assertEqual(len(children_data), 1)
        child_name, child_data = children_data[0]
        self.assertEqual(child_name, "child")
        child_data.pop("level")
        check_common(data["child"], child_data, child_level)

    def test_setup_nodes(self):
        """Test _setup_nodes()."""
        node_data = [
            build_node_data(time=float(i + 1), num_calls=i + 2) for i in range(7)
        ]
        node_data[0]["level"] = 0
        node_data[1]["level"] = 1
        node_data[2]["level"] = 2
        node_data[3]["level"] = 2
        node_data[4]["level"] = 1
        node_data[5]["level"] = 2
        node_data[6]["level"] = 2

        data = {
            "root": {
                **node_data[0],
                "node0": {
                    **node_data[1],
                    "section0": node_data[2],
                    "section1": node_data[3],
                },
                "node1": {
                    **node_data[4],
                    "section0": node_data[5],
                    "section1": node_data[6],
                },
            }
        }

        nodes, sections = PerfGraph._setup_nodes(deepcopy(data))

        self.assertIsInstance(nodes, dict)
        for k, v in nodes.items():
            self.assertIsInstance(k, int)
            self.assertIsInstance(v, PerfGraphNode)
        self.assertEqual(len(nodes), 7)

        self.assertIsInstance(sections, dict)
        for k, v in sections.items():
            self.assertIsInstance(k, str)
            self.assertIsInstance(v, PerfGraphSection)
        self.assertEqual(len(sections), 5)

        def check_node(name, i, num_children):
            node = nodes[i]
            self.assertEqual(node.name, name)
            self.assertEqual(node.self_time, node_data[i]["time"])
            self.assertEqual(node.num_calls, node_data[i]["num_calls"])
            self.assertEqual(node.level, node_data[i]["level"])
            self.assertEqual(len(node.children), num_children)
            self.assertEqual(node.root_node, nodes[0])
            return node

        # root
        root = check_node("root", 0, 2)
        self.assertIsNone(root.parent)
        # node0 and children
        node0 = check_node("node0", 1, 2)
        node0_section0 = check_node("section0", 2, 0)
        node0_section1 = check_node("section1", 3, 0)
        self.assertEqual(id(node0), id(node0_section0.parent))
        self.assertEqual(id(node0), id(node0_section1.parent))
        # node1 and children
        node1 = check_node("node1", 4, 2)
        node1_section0 = check_node("section0", 5, 0)
        node1_section1 = check_node("section1", 6, 0)
        self.assertEqual(id(node1), id(node1_section0.parent))
        self.assertEqual(id(node1), id(node1_section1.parent))

        def check_section(name, nodes):
            section = sections[name]
            self.assertEqual(len(section.nodes), len(nodes))
            for i, node in enumerate(nodes):
                self.assertEqual(id(section.nodes[i]), id(node))

        # root section
        check_section("root", [root])
        # node0 section
        check_section("node0", [node0])
        # node1 section
        check_section("node1", [node1])
        # section0 section
        check_section("section0", [node0_section0, node1_section0])
        # section1 section
        check_section("section1", [node0_section1, node1_section1])

    def test_root_node(self):
        """Test property root_node."""
        pg = build_perf_graph()
        self.assertIsNotNone(pg._root_node)
        self.assertEqual(id(pg._root_node), id(pg.root_node))

    def test_nodes(self):
        """Test property nodes."""
        pg = build_perf_graph()
        self.assertEqual(list(pg.nodes), list(pg._nodes.values()))

    def test_sections(self):
        """Test property sections."""
        pg = build_perf_graph()
        self.assertEqual(list(pg.sections), list(pg._sections.values()))

    def test_total_time(self):
        """Test property total_time."""
        pg = build_perf_graph()
        self.assertEqual(pg.root_node.total_time, pg.total_time)

    def test_get_node_by_id(self):
        """Test get_node_by_id()."""
        data = {"root": {**build_node_data(), "child": build_node_data()}}
        pg = PerfGraph(data)

        root = pg.get_node_by_id(0)
        self.assertIsInstance(root, PerfGraphNode)
        self.assertEqual(root.name, "root")

        child = pg.get_node_by_id(1)
        self.assertIsInstance(child, PerfGraphNode)
        self.assertEqual(child.name, "child")

        with self.assertRaisesRegex(KeyError, "Node does not exist with ID 2"):
            pg.get_node_by_id(2)

    def test_query_section(self):
        """Test query_section()."""
        pg = build_perf_graph()

        section = pg.query_section("root")
        assert section is not None
        self.assertIsInstance(section, PerfGraphSection)
        self.assertEqual(section.name, "root")

        self.assertIsNone(pg.query_section("foo"))

    def test_has_section(self):
        """Test has_section()."""
        pg = build_perf_graph()
        self.assertTrue(pg.has_section("root"))
        self.assertFalse(pg.has_section("foo"))

    def test_get_section(self):
        """Test get_section()."""
        pg = build_perf_graph()

        root = pg.get_section("root")
        self.assertIsInstance(root, PerfGraphSection)

        with self.assertRaisesRegex(KeyError, "Section does not exist with name 'foo'"):
            pg.get_section("foo")

    def test_get_heaviest_nodes(self):
        """Test get_heaviest_nodes()."""
        nodes = [
            build_test_node(time=1.0),
            build_test_node(time=100.0),
            build_test_node(time=0.01),
            build_test_node(time=200.0),
        ]
        pg = build_perf_graph()
        pg._nodes = {i: node for i, node in enumerate(nodes)}
        heaviest = pg.get_heaviest_nodes(3)
        self.assertIsInstance(heaviest, list)
        self.assertEqual(len(heaviest), 3)
        self.assertEqual(id(heaviest[0]), id(nodes[3]))
        self.assertEqual(id(heaviest[1]), id(nodes[1]))
        self.assertEqual(id(heaviest[2]), id(nodes[0]))

    def test_get_heaviest_sections(self):
        """Test get_heaviest_sections()."""
        sections = [
            build_test_section(nodes=[build_test_node(time=0.01)]),
            build_test_section(nodes=[build_test_node(time=100.0)]),
            build_test_section(nodes=[build_test_node(time=2.0)]),
            build_test_section(nodes=[build_test_node(time=50.0)]),
        ]
        pg = build_perf_graph()
        pg._sections = {str(i): section for i, section in enumerate(sections)}
        heaviest = pg.get_heaviest_sections(3)
        self.assertIsInstance(heaviest, list)
        self.assertEqual(len(heaviest), 3)
        self.assertEqual(id(heaviest[0]), id(sections[1]))
        self.assertEqual(id(heaviest[1]), id(sections[3]))
        self.assertEqual(id(heaviest[2]), id(sections[2]))

    @pytest.mark.moose
    def test_live(self):
        """Test building the PerfGraph with live PerfGraphReporter output."""
        input_contents = """
[Mesh/gmg]
    type = GeneratedMeshGenerator
    dim = 1
[]

[Problem]
    solve = false
[]

[Executioner]
    type = Steady
[]
"""

        with TemporaryDirectory() as tmp_dir:
            input_file = os.path.join(tmp_dir, "input.i")
            with open(input_file, "w") as f:
                f.write(input_contents)

            perf_graph_file = os.path.join(tmp_dir, "perf_graph.json")

            cmd = [
                self.moose_exe,
                "-i",
                "input.i",
                f"Outputs/perf_graph_json_file={perf_graph_file}",
            ]
            subprocess.run(cmd, cwd=tmp_dir, check=True)

            with open(perf_graph_file, "r") as f:
                reporter_data = json.load(f)

        data = reporter_data["time_steps"][-1]["perf_graph_json"]["graph"]
        pg = PerfGraph(data)
        self.assertIn(" (main)", pg.root_node.name)
        self.assertTrue(pg.root_node.has_child("MooseApp::run"))
