#!/usr/bin/env python3
#pylint: disable=missing-docstring
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
import shutil
import mooseutils
import chigger

class TestExodusReader(unittest.TestCase):
    """
    Test use of MooseDataFrame for loading/reloading csv files.
    """
    @classmethod
    def setUpClass(cls):
        """
        Copy test files.
        """
        cls.single = "{}_single.e".format(cls.__name__)
        shutil.copyfile(os.path.abspath('../input/mug_blocks_out.e'), cls.single)

        cls.vector = "{}_vector.e".format(cls.__name__)
        shutil.copyfile(os.path.abspath('../input/vector_out.e'), cls.vector)

        cls.multiple = "{}_multiple".format(cls.__name__)
        cls.testfiles = chigger.utils.copyAdaptiveExodusTestFiles(cls.multiple)
        cls.multiple += '.e'

    @classmethod
    def tearDownClass(cls):
        """
        Remove test files.
        """
        for fname in cls.testfiles:
            if os.path.exists(fname):
                os.remove(fname)
        if os.path.exists(cls.single):
            os.remove(cls.single)
        if os.path.exists(cls.vector):
            os.remove(cls.vector)

    def testRange(self):
        """
        Test reading of a single Exodus file.
        """
        reader = chigger.exodus.ExodusReader(self.single)
        source = chigger.exodus.ExodusSource(reader)

        # Ranges
        source.update(variable='aux_elem')
        rng = source.getRange()
        self.assertAlmostEqual(rng[0], 0.009018163476295126)
        self.assertAlmostEqual(rng[1], 9.991707387689374)

        source.update(variable='convected')
        rng = source.getRange()
        self.assertAlmostEqual(rng[0], 0.0)
        self.assertAlmostEqual(rng[1], 1.0)

        source.update(variable='diffused')
        rng = source.getRange()
        self.assertAlmostEqual(rng[0], 0.0)
        self.assertAlmostEqual(rng[1], 2.0)

        # Limit to blocks
        # When limiting by blocks and an accurate range is desired then "squeeze points" must be enabled; otherwise,
        # the un-limited range is utilized.
        reader.setOptions(squeeze=True)
        source.update(variable='diffused', block=['76'])
        rng = source.getRange()
        self.assertAlmostEqual(rng[0], 0.0)
        self.assertAlmostEqual(rng[1], 1.8851526891663235)

        reader.setOptions(squeeze=False)
        source.update()
        rng = source.getRange()
        self.assertAlmostEqual(rng[0], 0.0)
        self.assertAlmostEqual(rng[1], 2.0)

    def testExceptions(self):
        """
        Test exceptions.
        """
        with self.assertRaisesRegexp(mooseutils.MooseException, 'The supplied reader must be a "chigger.readers.ExodusReader", but a "int" was provided.'):
            chigger.exodus.ExodusSource(12345)



if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
