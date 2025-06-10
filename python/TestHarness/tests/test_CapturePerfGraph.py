#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from TestHarnessTestCase import TestHarnessTestCase
import os

class CapturePerfGraph(TestHarnessTestCase):
    def test(self):
        out = self.runTests('-i', 'always_ok', '--capture-perf-graph').results
        test = out['tests']['tests/test_harness']['tests']['always_ok']
        status = test['status']
        self.assertEqual(status['status'], 'OK')
        tester = test['tester']
        input_file = tester['input_file']
        test_dir = os.path.dirname(input_file)
        perf_graph_path = os.path.join(test_dir, 'metadata_perf_graph_always_ok.json')
        json_metadata = tester['json_metadata']
        self.assertEqual(perf_graph_path, json_metadata['perf_graph'])
        tester_output = test['output']['tester']
        self.assertIn('Checking JSON metadata', tester_output)
        self.assertIn(f'perf_graph ({perf_graph_path}): exists and is valid', tester_output)
