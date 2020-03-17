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
import json
import io
from contextlib import redirect_stdout
# so we can find our libraries, no matter how we're called
findbin = os.path.dirname(os.path.realpath(sys.argv[0]))
sys.path.append(os.path.join(findbin, "../"))
import dbutils

class TestDBUtils(unittest.TestCase):
    """
    Test that the functions in dbutils work correctly
    """

    def readJSONDatabase(self):
        """
        Read the database
        """
        with open('./testdata/moose_testdb.json', 'r') as dbfile:
            moosedb = json.load(dbfile)

        self.db = moosedb

    def testsecondarySpeciesContainingBasis(self):
        """
        Test that the species info is correctly returned
        """

        self.readJSONDatabase()

        gold = ['CO2(aq)', 'CO3--', 'CaCO3']
        output = dbutils.secondarySpeciesContainingBasis(self.db, 'secondary species', ['HCO3-'])
        self.assertEqual(output, gold)

    def testprintSpeciesInfo(self):
        """
        Test that the species info is correctly returned
        """

        self.readJSONDatabase()

        # Test a basis species
        gold = "Ca++:\n  type: basis species\n  elements:  {'Ca': '1.000'}\n  charge:  2\n  radius:  6.0\n  molecular weight:  40.0800\n"

        # Capture the information printed out by printSpeciesInfo()
        f = io.StringIO()
        with redirect_stdout(f):
            dbutils.printSpeciesInfo(self.db, 'Ca++')

        output = f.getvalue()
        self.assertEqual(output, gold)

        # Test a secondary species
        gold = "CaCO3:\n  type: secondary species\n  species:  {'Ca++': '1.000', 'HCO3-': '1.000', 'H+': '-1.000'}\n  charge:  0\n  radius:  4.0\n  molecular weight:  100.0892\n  logk:  ['7.5520', '7.1280', '6.7340', '6.4350', '6.1810', '5.9320', '5.5640', '4.7890']\n"

        # Capture the information printed out by printSpeciesInfo()
        f = io.StringIO()
        with redirect_stdout(f):
            dbutils.printSpeciesInfo(self.db, 'CaCO3')

        output = f.getvalue()
        self.assertEqual(output, gold)

    def testprintEquilibriumReaction(self):
        """
        Test that the equilibrium reactions are correctly returned
        """

        self.readJSONDatabase()

        gold = "CaCO3 = 1.000 Ca++ + 1.000 HCO3- -1.000 H+\n"
        # Capture the information printed out
        f = io.StringIO()
        with redirect_stdout(f):
            dbutils.printEquilibriumReaction(self.db, 'CaCO3')

        output = f.getvalue()
        self.assertEqual(output, gold)

    def testprintMineralReaction(self):
        """
        Test that the mineral reactions are correctly returned
        """

        self.readJSONDatabase()

        gold = "Calcite = 1.000 Ca++ + 1.000 HCO3- -1.000 H+\n"
        # Capture the information printed out
        f = io.StringIO()
        with redirect_stdout(f):
            dbutils.printMineralReaction(self.db, 'Calcite')

        output = f.getvalue()
        self.assertEqual(output, gold)

    def testprintGasReaction(self):
        """
        Test that the gas reactions are correctly returned
        """

        self.readJSONDatabase()

        gold = "CH4(g) = 1.000 CH4(aq)\n"
        # Capture the information printed out
        f = io.StringIO()
        with redirect_stdout(f):
            dbutils.printGasReaction(self.db, 'CH4(g)')

        output = f.getvalue()
        self.assertEqual(output, gold)

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2, buffer=True, exit=False)
