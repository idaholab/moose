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
import sys
import unittest
# so we can find our libraries, no matter how we're called
findbin = os.path.dirname(os.path.realpath(sys.argv[0]))
sys.path.append(os.path.join(findbin, "../"))
from readers import eq36_reader
from dbclass import ThermoDB

class TestGWBReader(unittest.TestCase):
    """
    Test that the Geochemist WorkBench database reader functions correctly
    """

    def readDatabase(self):
        """
        Read the database
        """
        with open('testdata/eq36testdata.dat', 'r') as dbfile:
            dblist = dbfile.readlines()

        # Parse the database
        self.db = eq36_reader.readDatabase(dblist)

    def testTemperature(self):
        """
        Test that the temperatures are correctly parsed
        """
        self.readDatabase()
        gold = [0.0100, 25.0000, 60.0000, 100.0000, 150.0000, 200.0000, 250.0000, 300.0000]

        self.assertEqual(self.db.temperatures, gold)

    def testPressure(self):
        """
        Test that the pressures are correctly parsed
        """
        self.readDatabase()
        gold = [1.0132, 1.0132, 1.0132, 1.0132, 4.7572, 15.5365, 39.7365, 85.8378]

        self.assertEqual(self.db.pressures, gold)

    def testActivityModel(self):
        """
        Test that the activity model is set correctly
        """
        self.readDatabase()
        gold = 'debye-huckel'
        adhgold = [0.4939, 0.5114, 0.5465, 0.5995, 0.6855, 0.7994, 0.9593, 1.2180]
        bdhgold = [0.3253, 0.3288, 0.3346, 0.3421, 0.3525, 0.3639, 0.3766, 0.3925]
        bdotgold = [0.0394, 0.0410, 0.0438, 0.0460, 0.0470, 0.0470, 0.0340, 0.0000]

        self.assertEqual(self.db.activity_model, gold)
        self.assertEqual(self.db.adh, adhgold)
        self.assertEqual(self.db.bdh, bdhgold)
        self.assertEqual(self.db.bdot, bdotgold)


    def testFugacityModel(self):
        """
        Test that the fugacity model is set correctly
        """
        self.readDatabase()
        gold = None

        self.assertEqual(self.db.fugacity_model, gold)

    def testLogkModel(self):
        """
        Test that the equilibrium constant model is set correctly
        """
        self.readDatabase()
        gold = 'maier-kelly'
        eqngold = 'a_0 ln(T) + a_1 + a_2 T + a_3 / T + a_4 / T^2'

        self.assertEqual(self.db.logk_model, gold)
        self.assertEqual(self.db.logk_model_eqn, eqngold)


    def testNeutralSpecies(self):
        """
        Test that the neutral species coefficients are correctly parsed
        """
        self.readDatabase()
        gold = {'co2': {'(coefficients': [-1.0312, 0.0012806, 255.9, 0.4445, -0.001606]}}

        self.assertDictEqual(self.db.neutral_species, gold)

    def testElements(self):
        """
        Test that the elements are correctly parsed
        """
        self.readDatabase()
        gold = {'O': {'molecular weight': 15.99940},
                'Ag': {'molecular weight': 107.86820}}

        self.assertDictEqual(self.db.elements, gold)

    def testBasisSpecies(self):
        """
        Test that the basis species are correctly parsed
        """
        self.readDatabase()
        gold = {'H2O': {'charge': 0.0, 'radius': 3.0, 'molecular weight': 18.015,
                         'elements': {'H': 2.0000, 'O': 1.0000}},
                'Ag+': {'charge': 1.0, 'radius': 2.5, 'molecular weight': 107.868,
                                  'elements': {'Ag': 1.0000}}}

        self.assertDictEqual(self.db.basis_species, gold)

    def testSecondarySpecies(self):
        """
        Test that the secondary species are correctly parsed
        """
        self.readDatabase()
        gold = {'(NH4)2Sb2S4(aq)': {'charge': 0.0, 'radius': 3.0, 'molecular weight': 407.841,
                         'elements': {'H': 8.0000, 'N': 2.0000, 'S': 4.0000, 'Sb': 2.0000},
                         'species': {'H2O': -6.0000, 'NH3(aq)': 2.0000, 'Sb(OH)3(aq)': 2.0000,
                                     'H+': 4.0000, 'HS-': 4.0000},
                         'logk': [-74.5361, -67.6490, -59.8877, -52.5457, -45.0674, -38.8180,  -33.1941, -28.0323],
                         'note': 'Missing array values in original database have been filled using a maier-kelly fit. Original values are [-74.5361, -67.6490, -59.8877, -52.5457, -45.0674, -38.8180, 500.0000, 500.0000]'}}

        self.assertDictEqual(self.db.secondary_species, gold)

    def testMineralSpecies(self):
        """
        Test that the mineral species are correctly parsed
        """
        self.readDatabase()
        gold = {'Calcite': {'molar volume': 36.934,
                            'molecular weight': 100.087,
                            'elements': {'C': 1.0000, 'Ca': 1.0000, 'O': 3.0000},
                            'species': {'H+': -1.0000, 'Ca++': 1.0000, 'HCO3-': 1.0000},
                            'logk': [2.2257, 1.8487, 1.3330, 0.7743, 0.0999, -0.5838, -1.3262, -2.2154]}}

        self.assertDictEqual(self.db.mineral_species, gold)

    def testGasSpecies(self):
        """
        Test that the gas species are correctly parsed
        """
        self.readDatabase()
        gold = {'Ag(g)': {'species': {'H+': -1.0000, 'O2(g)': -0.2500, 'Ag+': 1.0000, 'H2O': 0.5000},
                          'elements': {'Ag': 1.0000},
                          'molecular weight': 107.868,
                          'logk': [55.5420, 50.3678, 44.4606, 39.1093, 33.8926, 29.8196, 26.2832, 23.1649],
                          'note': 'Missing array values in original database have been filled using a maier-kelly fit. Original values are [55.5420, 50.3678, 44.4606, 39.1093, 33.8926, 29.8196, 500.0000, 500.0000]'}}

        self.assertDictEqual(self.db.gas_species, gold)

    def testRedoxCouples(self):
        """
        Test that the redox couples are correctly parsed
        """
        self.readDatabase()
        gold = {'HS-': {'species': {'SO4--': 1.0000, 'H+': 1.0000, 'O2(g)': -2.0000},
                               'elements': {'H': 1.0000, 'S': 1.0000},
                               'charge': -1.0,
                               'radius': 3.5,
                               'molecular weight': 33.074,
                               'logk': [146.7859, 132.5203, 116.0105, 100.8144, 85.7147, 73.6540, 63.7280, 55.2988]}}

        self.assertDictEqual(self.db.redox_couples, gold)

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2, buffer=True, exit=False)
