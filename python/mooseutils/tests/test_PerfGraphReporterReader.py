#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import copy, os, unittest
from mooseutils.PerfGraphReporterReader import PerfGraphReporterReader, PerfGraphNode
from mooseutils.ReporterReader import ReporterReader

class TestPerfGraphReporterReader(unittest.TestCase):
    """
    Test use of PerfGraphReporterReader for loading PerfGraphReporter data.
    """

    def setUp(self):
        self._file = os.path.abspath('../../../test/tests/reporters/perf_graph_reporter/gold/perf_graph_reporter_json.json')

        reader = ReporterReader(self._file)
        reader.update(reader.times()[-1])
        self._data = reader[('perf_graph', 'graph')]
        self._root_node = self._data['MooseTestApp (main)']

    def test(self):
        def totalTime(node_data):
            return node_data['time'] + childrenTime(node_data)

        def totalMemory(node_data):
            return node_data['memory'] + childrenMemory(node_data)

        def childrenTime(node_data):
            children_time = 0
            for entry in node_data.values():
                if type(entry) == dict:
                    children_time += totalTime(entry)
            return children_time

        def childrenMemory(node_data):
            children_memory = 0
            for entry in node_data.values():
                if type(entry) == dict:
                    children_memory += totalMemory(entry)
            return children_memory

        pgrr = PerfGraphReporterReader(self._file)

        self.assertNotEqual(None, pgrr.rootNode())
        self.assertAlmostEqual(100, pgrr.rootNode().percentTime())
        self.assertAlmostEqual(100, pgrr.rootNode().percentMemory())

        root_node_name = list(self._data.keys())[0]
        self.assertEqual(root_node_name, pgrr.rootNode().name())

        root_node_data = list(self._data.values())[0]
        root_node_time = totalTime(root_node_data)
        root_node_memory = totalMemory(root_node_data)

        sections = {}

        def verify_node(node):
            print(node)
            print(node.info())

            node_data = self._data
            for name in node.path():
                self.assertIn(name, node_data)
                node_data = node_data[name]

            self.assertEqual(node._nodes, [node])
            self.assertEqual(node.name(), node.path()[-1])
            self.assertEqual(pgrr.node(node.path()), node)
            self.assertEqual(node.name(), node.section().name())
            self.assertIn(node, node.section().nodes())
            self.assertEqual(pgrr.rootNode(), node.rootNode())

            self.assertEqual(node.level(), node_data['level'])
            self.assertEqual(node.numCalls(), node_data['num_calls'])

            self.assertEqual(node.selfTime(), node_data['time'])
            self.assertEqual(node.childrenTime(), childrenTime(node_data))
            self.assertEqual(node.totalTime(), totalTime(node_data))
            self.assertEqual(node.percentTime(), totalTime(node_data) * 100 / root_node_time)

            self.assertEqual(node.selfMemory(), node_data['memory'])
            self.assertEqual(node.childrenTime(), childrenTime(node_data))
            self.assertEqual(node.totalTime(), totalTime(node_data))
            self.assertEqual(node.percentMemory(), totalMemory(node_data) * 100 / root_node_memory)

            if node.name() not in sections:
                sections[node.name()] = []
            sections[node.name()].append(node_data)

            for child in node.children():
                self.assertEqual(child.parent(), node)

        # Verify nodes
        pgrr.recurse(verify_node)

        # Verify sections
        for name, data in sections.items():
            section = pgrr.section(name)
            print(section)
            print(section.info())

            self.assertEqual(name, section.name())
            self.assertEqual(len(data), len(section.nodes()))

            for node in section.nodes():
                self.assertEqual(section.node(node.path()), node)
                self.assertEqual(node.section(), section)
                self.assertEqual(node.level(), section.level())
                self.assertEqual(node.name(), section.name())

            self.assertEqual(section.numCalls(), sum([node.numCalls() for node in section.nodes()]))

            self.assertAlmostEqual(section.selfTime(), sum([node.selfTime() for node in section.nodes()]))
            self.assertAlmostEqual(section.totalTime(), sum([node.totalTime() for node in section.nodes()]))
            self.assertAlmostEqual(section.childrenTime(), sum([node.childrenTime() for node in section.nodes()]))

            self.assertAlmostEqual(section.selfMemory(), sum([node.selfMemory() for node in section.nodes()]))
            self.assertAlmostEqual(section.totalMemory(), sum([node.totalMemory() for node in section.nodes()]))
            self.assertAlmostEqual(section.childrenMemory(), sum([node.childrenMemory() for node in section.nodes()]))

        heaviest_nodes = pgrr.heaviestNodes(2)
        self.assertEqual(len(heaviest_nodes), 2)
        self.assertEqual(heaviest_nodes[0].name(), 'MooseApp::setupOptions')
        self.assertEqual(heaviest_nodes[1].name(), 'MooseTestApp (main)')

        heaviest_mem_nodes = pgrr.heaviestNodes(2, memory=True)
        self.assertEqual(len(heaviest_mem_nodes), 2)
        self.assertEqual(heaviest_mem_nodes[0].name(), 'MooseTestApp (main)')
        self.assertEqual(heaviest_mem_nodes[1].name(), 'FEProblem::solve')

        heaviest_sections = pgrr.heaviestSections(2)
        self.assertEqual(len(heaviest_sections), 2)
        self.assertEqual(heaviest_sections[0].name(), 'MooseApp::setupOptions')
        self.assertEqual(heaviest_sections[1].name(), 'MooseTestApp (main)')

        heaviest_mem_sections = pgrr.heaviestSections(2, memory=True)
        self.assertEqual(len(heaviest_sections), 2)
        self.assertEqual(heaviest_mem_sections[0].name(), 'MooseTestApp (main)')
        self.assertEqual(heaviest_mem_sections[1].name(), 'Console::outputStep')

    def testExceptions(self):
        pgrr = PerfGraphReporterReader(self._file)

        with self.assertRaisesRegex(TypeError, '"num" should be an int'):
            pgrr.heaviestNodes('foo')
        with self.assertRaisesRegex(ValueError, '"num" should be >= 1'):
            pgrr.heaviestNodes(-1)
        with self.assertRaisesRegex(TypeError, '"num" should be an int'):
            pgrr.heaviestSections('foo')
        with self.assertRaisesRegex(ValueError, '"num" should be >= 1'):
            pgrr.heaviestSections(-1)

    def testPerfGraphNodeExceptions(self):
        with self.assertRaisesRegex(TypeError, 'parent is not of type "PerfGraphNode"'):
            PerfGraphNode(0, self._root_node, 'foo')

        for key in ['level', 'memory', 'num_calls', 'time']:
            with self.assertRaisesRegex(Exception, 'Entry missing key "{}"'.format(key)):
                data = copy.deepcopy(self._root_node)
                del data[key]
                PerfGraphNode('foo', data, None)

            with self.assertRaisesRegex(Exception, 'Key "{}" in node entry is not the required type'.format(key)):
                data = copy.deepcopy(self._root_node)
                data[key] = None
                PerfGraphNode('foo', data, None)

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2, buffer=True)
