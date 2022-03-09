#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import unittest
import subprocess
import mooseutils

class TestReporterReader(unittest.TestCase):
    """
    Test use of ReporterReader for loading/reloading json files.
    """

    def setUp(self):
        """
        Define the test filenames.
        """
        self._basicfile = os.path.abspath('../../../test/tests/reporters/constant_reporter/gold/constant_reporter_out.json')
        self._timefile = os.path.abspath('../../../test/tests/reporters/accumulated_reporter/gold/accumulate_reporter_out.json')
        self._onepertimefile = os.path.abspath('../../../test/tests/outputs/json/one_file_per_timestep/gold/json_out_*.json')
        self._partsfile = os.path.abspath('../../../test/tests/reporters/mesh_info/gold/mesh_info_out.json')

    def testBasic(self):
        """
        Test functionality with a basic json output
        """

        # Test basic read
        data = mooseutils.ReporterReader(self._basicfile)

        # Test __getitem__
        self.assertEqual(data[('constant', 'str_vec')], ['ten', 'eleven', 'twelve', 'thirteen'])
        self.assertEqual(data[[('constant', 'num_2'), ('constant', 'int_3')]], [5.0, -3])

        # Test __bool__
        self.assertTrue(data)

        # Test __contains__
        self.assertTrue(('constant', 'int_vec') in data)
        self.assertFalse(('foo', 'bar') in data)
        self.assertFalse(('foo', 'int_vec') in data)
        self.assertFalse(('constant', 'bar') in data)

        # Test clear()
        tmp = mooseutils.ReporterReader(self._basicfile)
        tmp.clear()
        self.assertFalse(tmp)

        # Test variables()
        vars = data.variables()
        self.assertEqual(len(vars), 14)
        self.assertEqual(vars[0], ('constant', 'dofid_1'))
        self.assertEqual(vars[1], ('constant', 'dofid_2'))
        self.assertEqual(vars[2], ('constant', 'dofid_3'))
        self.assertEqual(vars[3], ('constant', 'dofid_vec'))
        self.assertEqual(vars[4], ('constant', 'int_1'))
        self.assertEqual(vars[5], ('constant', 'int_2'))
        self.assertEqual(vars[6], ('constant', 'int_3'))
        self.assertEqual(vars[7], ('constant', 'int_vec'))
        self.assertEqual(vars[8], ('constant', 'num_1'))
        self.assertEqual(vars[9], ('constant', 'num_2'))
        self.assertEqual(vars[10], ('constant', 'str'))
        self.assertEqual(vars[11], ('constant', 'str_vec'))
        self.assertEqual(vars[12], ('constant', 'vec_1'))
        self.assertEqual(vars[13], ('constant', 'vec_2'))

        # Test repr()
        output, imports = data.repr()
        output += ["print('Success' if data else 'Fail')"]
        script = '{}_repr.py'.format(self.__class__.__name__)
        with open(script, 'w') as fid:
            fid.write('\n'.join(imports))
            fid.write('\n'.join(output))
        self.assertTrue(os.path.exists(script))
        out = subprocess.check_output(['python', script])
        self.assertIn('Success', out.decode())
        os.remove(script)

    def testTimeData(self):
        """
        Test functionality with time dependent data
        """

        data = mooseutils.ReporterReader(self._timefile)

        # Test __getitem__
        self.assertEqual(data[('accumulate', 'rep:str')], ["two", "two", "two", "two", "two", "two"])

        # Test times()
        self.assertEqual(data.times(), [0.0, 1.0, 2.0, 3.0, 4.0, 5.0])

        # Test update()
        pp = []
        for time in data.times():
            pp.append(time)
            data.update(time)
            self.assertEqual(data[('accumulate', 'pp:value')], pp)

    def testTimeData(self):
        """
        Test functionality with one file per timestep
        """

        data = mooseutils.ReporterReader(self._onepertimefile)
        self.assertEqual(data.times(), [0.0, 1.0, 2.0, 3.0])

    def testParts(self):
        """
        Test functionality for reading different parts
        """

        data = mooseutils.ReporterReader(self._partsfile)
        self.assertEqual(data._data['number_of_parts'], 2)
        self.assertEqual(data._data['part'], 0)

        data.update(part=1)
        self.assertEqual(data._data['part'], 1)

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2, buffer=True)
