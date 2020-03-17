#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import unittest
from readers import gwb_reader
from dbclass import ThermoDB

class TestGWBReader(unittest.TestCase):
    """
    Test that the Geochemist WorkBench database reader functions correctly
    """

    def readDatabase(self):
        """
        Read the database
        """
        with open('testdata/gwbtestdata.dat', 'r') as dbfile:
            dblist = dbfile.readlines()

        # Parse the database
        self.db = gwb_reader.readDatabase(dblist)

    def testTemperature(self):
        """
        Test that the temperatures are correctly parsed
        """
        self.readDatabase()
        gold = ['0.0000', '25.0000', '60.0000', '100.0000',
                '150.0000', '200.0000', '250.0000', '300.0000']

        self.assertEqual(self.db.temperatures, gold)

    def testPressure(self):
        """
        Test that the pressures are correctly parsed
        """
        self.readDatabase()
        gold = ['1.0134', '1.0134', '1.0134', '1.0134',
                '4.7600', '15.5490', '39.7760', '85.9270']

        self.assertEqual(self.db.pressures, gold)

    def testActivityModel(self):
        """
        Test that the activity model is set correctly
        """
        self.readDatabase()
        gold = 'debye-huckel'
        adhgold = ['.4913', '.5092', '.5450', '.5998',
                   '.6898', '.8099', '.9785', '1.2555']
        bdhgold = ['.3247', '.3283', '.3343', '.3422',
                   '.3533', '.3655', '.3792', '.3965']
        bdotgold = ['.0174', '.0410', '.0440', '.0460',
                    '.0470', '.0470', '.0340', '0.0000']

        self.assertEqual(self.db.activity_model, gold)
        self.assertEqual(self.db.adh, adhgold)
        self.assertEqual(self.db.bdh, bdhgold)
        self.assertEqual(self.db.bdot, bdotgold)


    def testFugacityModel(self):
        """
        Test that the fugacity model is set correctly
        """
        self.readDatabase()
        gold = 'tsonopoulos'

        self.assertEqual(self.db.fugacity_model, gold)

    def testNeutralSpecies(self):
        """
        Test that the neutral species coefficients are correctly parsed
        """
        self.readDatabase()
        gold = {'co2': {'c co2 1': ['.1224', '.1127', '.09341', '.08018',
                                    '.08427', '.09892', '.1371', '.1967'],
                        'c co2 2': ['-.004679', '-.01049', '-.0036', '-.001503',
                                    '-.01184', '-.0104', '-.007086', '-.01809']},
                'h2o': {'c h2o 1': ['500.0000', '1.45397', '500.0000', '1.5551',
                                    '1.6225', '500.0000', '500.0000', '500.0000']}}

        self.assertDictEqual(self.db.neutral_species, gold)

    def testElements(self):
        """
        Test that the elements are correctly parsed
        """
        self.readDatabase()
        gold = {'Ag': {'name': 'Silver', 'molecular weight': '107.8680'},
                'Al': {'name': 'Aluminum', 'molecular weight': '26.9815'}}

        self.assertDictEqual(self.db.elements, gold)

    def testBasisSpecies(self):
        """
        Test that the basis species are correctly parsed
        """
        self.readDatabase()
        gold = {'H2O': {'charge': '0', 'radius': '0.0', 'molecular weight': '18.0152',
                         'elements': {'H': '2.000', 'O': '1.000'}},
                'Ag+': {'charge': '1', 'radius': '2.5', 'molecular weight': '107.8680',
                                  'elements': {'Ag': '1.000'}}}

        self.assertDictEqual(self.db.basis_species, gold)

    def testSecondarySpecies(self):
        """
        Test that the secondary species are correctly parsed
        """
        self.readDatabase()
        gold = {'AgSO4-': {'charge': '-1', 'radius': '4.0', 'molecular weight': '203.9256',
                         'species': {'Ag+': '1.000', 'SO4--': '1.000'},
                         'logk': ['-1.2200', '-1.3000', '-1.4400', '-1.6200',
                                  '-1.8900', '-2.2300', '-2.6900', '-3.4400']}}

        self.assertDictEqual(self.db.secondary_species, gold)

    def testMineralSpecies(self):
        """
        Test that the mineral species are correctly parsed
        """
        self.readDatabase()
        gold = {'Akermanite': {'molar volume': '92.8100', 'molecular weight': '272.6318',
                               'species': {'H2O': '3.000', 'Ca++': '2.000', 'Mg++': '1.000',
                                           'SiO2(aq)': '2.000', 'H+': '-6.000'},
                               'logk': ['49.8221', '45.1875', '39.7039', '34.7089',
                                        '29.8686', '26.0806', '22.9032', '19.7991']},
                'Bieberite': {'molar volume': '144.3000', 'molecular weight': '281.0972',
                              'species': {'Co++': '1.000', 'SO4--': '1.000', 'H2O': '7.000'},
                              'logk': ['-2.7087', '-2.4973', '-2.3236', '-2.2232',
                                          '-2.2194', '-2.3543', '-2.7021', '-3.5004']},
                                  }

        self.assertDictEqual(self.db.mineral_species, gold)

    def testGasSpecies(self):
        """
        Test that the gas species are correctly parsed
        """
        self.readDatabase()
        gold = {'CH4(g)': {'species': {'CH4(aq)': '1.000'},
                           'molecular weight': '16.0426',
                           'chi': ['-537.779', '1.54946', '-.000927827', '1.20861', '-.00370814', '3.33804e-6'],
                           'Pcrit': '46.0',
                           'Tcrit': '190.4',
                           'omega': '.011',
                           'logk': ['-2.6487', '-2.8202', '-2.9329', '-2.9446',
                                    '-2.9163', '-2.7253', '-2.4643', '-2.1569']},
                'N2(g)': {'species': {'N2(aq)': '1.000'},
                          'molecular weight': '28.0134',
                          'Pcrit': '33.9',
                          'Tcrit': '126.2',
                          'omega': '.039',
                          'logk': ['-2.9620', '-3.1848', '-3.3320', '-3.2902',
                                   '-3.1631', '-2.9499', '-2.7827', '-2.3699']}}

        self.assertDictEqual(self.db.gas_species, gold)

    def testRedoxCouples(self):
        """
        Test that the redox couples are correctly parsed
        """
        self.readDatabase()
        gold = {'(O-phth)--': {'species': {'H2O': '-5.000', 'HCO3-': '8.000',
                                           'H+': '6.000', 'O2(aq)': '-7.500'},
                               'charge': '-2',
                               'radius': '4.0',
                               'molecular weight': '164.1172',
                               'logk': ['594.3211', '542.8292', '482.3612', '425.9738',
                                        '368.7004', '321.8658', '281.8216', '246.4849']},
                 'Am++++': {'species': {'H2O': '-.500', 'H+': '1.000',
                                        'Am+++': '1.000', 'O2(aq)': '.250'},
                            'charge': '4',
                            'radius': '11.0',
                            'molecular weight': '241.0600',
                            'logk': ['18.7967', '18.0815', '17.2698', '16.5278',
                                     '15.8024', '15.2312', '14.7898', '14.4250']}}

        self.assertDictEqual(self.db.redox_couples, gold)

    def testOxidess(self):
        """
        Test that the oxides are correctly parsed
        """
        self.readDatabase()
        gold = {'Cu2O': {'species': {'H+': '-2.000', 'Cu+': '2.000', 'H2O': '1.000'},
                         'molecular weight': '143.0929'}}

        self.assertDictEqual(self.db.oxides, gold)

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2, buffer=True, exit=False)
