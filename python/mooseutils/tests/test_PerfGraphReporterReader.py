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
        self._file = 'perf_graph_reporter_out.json'

        reader = ReporterReader(self._file)
        reader.update(reader.times()[-1])
        self._data = reader[('perf_graph', 'perf_graph')]['graph']

    def childrenTime(self, node_data):
        val = 0
        for entry in self._data:
            if 'parent_id' in entry and entry['parent_id'] == node_data['id']:
                val += self.totalTime(entry)
        return val

    def childrenMemory(self, node_data):
        val = 0
        for entry in self._data:
            if 'parent_id' in entry and entry['parent_id'] == node_data['id']:
                val += self.totalMemory(entry)
        return val

    def totalTime(self, node_data):
        return node_data['time'] + self.childrenTime(node_data)

    def totalMemory(self, node_data):
        return node_data['memory'] + self.childrenMemory(node_data)

    def test(self):
        # Test both file and raw separately
        for args in [{'file': self._file}, {'raw': self._data}]:
            pgrr = PerfGraphReporterReader(**args)

            # Find the root in the data
            root_node_data = None
            for entry in self._data:
                if 'parent_id' not in entry:
                    root_node_data = entry
                    self.assertEqual(root_node_data['id'], pgrr.rootNode().id())
                    self.assertEqual(pgrr.rootNode().parent(), None)
            self.assertNotEqual(root_node_data, None)
            root_time_total = self.totalTime(root_node_data)
            root_memory_total = self.totalMemory(root_node_data)

            # Should have the same number of nodes overall
            self.assertEqual(len(pgrr.nodes()), len(self._data))

            sections = []
            all_children = {}
            all_parents = {}

            # Verify each node in json
            for node_data in self._data:
                node = pgrr.node(node_data['id'])
                self.assertEqual(node.id(), node_data['id'])
                self.assertEqual(node.level(), node_data['level'])
                self.assertEqual(node.name(), node_data['name'])
                self.assertEqual(node.numCalls(), node_data['num_calls'])

                self.assertEqual(node.selfTime(), node_data['time'])
                self.assertEqual(node.childrenTime(), self.childrenTime(node_data))
                self.assertEqual(node.totalTime(), self.totalTime(node_data))
                self.assertEqual(node.percentTime(), self.totalTime(node_data) * 100 / root_time_total)

                self.assertEqual(node.selfMemory(), node_data['memory'])
                self.assertEqual(node.childrenMemory(), self.childrenMemory(node_data))
                self.assertEqual(node.totalMemory(), self.totalMemory(node_data))
                self.assertEqual(node.percentMemory(), self.totalMemory(node_data) * 100 / root_memory_total)

                if 'parent_id' in node_data:
                    self.assertEqual(node_data['parent_id'], node.parent().id())
                    self.assertEqual(node.rootNode(), pgrr.rootNode())

                children_ids_data = node_data.get('children_ids', [])
                self.assertEqual(len(node.children()), len(children_ids_data))
                for child in node.children():
                    self.assertIn(child.id(), children_ids_data)

                self.assertIn(node_data['name'], pgrr.sections())
                self.assertIn(node, pgrr.section(node_data['name']))
                if node_data['name'] not in sections:
                    sections.append(node_data['name'])

            self.assertEqual(len(sections), len(pgrr.sections()))

    def testExceptions(self):
        with self.assertRaisesRegex(Exception, 'Must provide either "file" or "raw"'):
            PerfGraphReporterReader()

        with self.assertRaisesRegex(Exception, 'Cannot provide both "file" and "raw"'):
            PerfGraphReporterReader(file='foo', raw='bar')

        with self.assertRaisesRegex(Exception, '"part" is not used with "raw"'):
            PerfGraphReporterReader(raw='foo', part=1)

        with self.assertRaisesRegex(Exception, 'Multiple root nodes found'):
            data = copy.deepcopy(self._data)
            del data[1]['parent_id']
            PerfGraphReporterReader(raw=data)

    def testPerfGraphNodeExceptions(self):
        with self.assertRaisesRegex(Exception, 'Entry missing ID'):
            data = copy.deepcopy(self._data)
            del data[0]['id']
            PerfGraphNode(0, None, data)

        with self.assertRaisesRegex(Exception, 'Duplicate ID 0 found'):
            data = copy.deepcopy(self._data)
            data[1]['id'] = 0
            PerfGraphNode(0, None, data)

        with self.assertRaisesRegex(Exception, 'Failed to find node with ID 123456'):
            PerfGraphNode(123456, None, data)

        with self.assertRaisesRegex(Exception, 'parent is not of type "PerfGraphNode"'):
            PerfGraphNode(0, 'foo', self._data)

        for key in ['level', 'memory', 'name', 'num_calls', 'time', 'parent_id']:
            if key in self._data[0]:
                with self.assertRaisesRegex(Exception, 'Entry missing key "{}"'.format(key)):
                    data = copy.deepcopy(self._data)
                    del data[0][key]
                    PerfGraphNode(0, None, data)

            with self.assertRaisesRegex(Exception, 'Key "{}" in node entry is not the required type'.format(key)):
                data = copy.deepcopy(self._data)
                data[0][key] = None
                PerfGraphNode(0, None, data)

        with self.assertRaisesRegex(Exception, 'Key "children_ids" in node entry is not the required type'):
            data = copy.deepcopy(self._data)
            data[0]['children_ids'] = ['foo']
            PerfGraphNode(0, None, data)

        with self.assertRaisesRegex(Exception, 'Key "foo" in node entry is invalid'):
            data = copy.deepcopy(self._data)
            data[0]['foo'] = 'bar'
            PerfGraphNode(0, None, data)


if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2, buffer=True)
