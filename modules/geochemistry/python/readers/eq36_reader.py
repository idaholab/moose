#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from dbclass import ThermoDB
import readers.reader_utils as reader_utils

def readDatabase(dblist):
    """
    Read an EQ3/6 database file
    """

    dataset = []
    activity_model = None
    fugacity_model = None
    logk_model = 'maier-kelly'
    logk_model_eqn = 'a_0 ln(T) + a_1 + a_2 T + a_3 / T + a_4 / T^2'
    temperatures = []
    pressures = []
    adh = []
    bdh = []
    bdot = []
    elements = {}
    neutral_species = {}
    basis_species = {}
    redox_couples = {}
    secondary_species = {}
    free_electron = {}
    mineral_species = {}
    gas_species = {}
    oxides = {}

    missing_value = '500.0000'
    line = 0
    parsing_header = True

    # Helper function for interpolating missing array values
    # (to reduce number of paramters required during use)
    def fillValues(vals):
        return reader_utils.fillMissingValues(temperatures, vals, logk_model, missing_value)

    while line < len(dblist):

        # Read the header information
        while parsing_header:
            dataset.append(dblist[line])

            # if dblist[line].split(':')[0] == 'Output package':
            if dblist[line].strip().startswith('Output package'):
                assert dblist[line].split(':')[1].strip() == 'eq3', 'EQ3/6 reader detected database in wrong format'

            if dblist[line].strip().startswith('Data set'):
                if dblist[line].split(':')[1].strip() in ['com', 'sup']:
                    activity_model = 'debye-huckel'
                else:
                    print('Activity model in supplied databse not supported')
                    exit()

            line = line+1

            if dblist[line].strip().startswith('+--'):
                parsing_header = False
                line = line+1

        # Read the miscellaneous data
        if dblist[line].strip().startswith('Miscellaneous') and dblist[line+1].strip().startswith('+--'):
            reading_miscellaneous = True
            line = line+2

            while not dblist[line].strip().startswith('+--'):
                if dblist[line].strip().startswith('temperatures'):
                    line = line+1
                    while dblist[line].strip()[0].isnumeric():
                        temperatures.extend([float(item) for item in dblist[line].split()])
                        line = line+1

                if dblist[line].strip().startswith('pressures'):
                    line = line+1
                    while dblist[line].strip()[0].isnumeric():
                        pressures.extend([float(item) for item in dblist[line].split()])
                        line = line+1

                if activity_model == 'debye-huckel':
                    # Read Debye-Huckel activity coefficients
                    if 'debye huckel a' in dblist[line]:
                        line = line+1
                        while len(adh) < len(temperatures):
                            adh.extend([float(item) for item in dblist[line].split()])
                            line = line+1

                    if 'debye huckel b' in dblist[line]:
                        line = line+1
                        while len(bdh) < len(temperatures):
                            bdh.extend([float(item) for item in dblist[line].split()])
                            line = line+1

                    if 'bdot' in dblist[line]:
                        line = line+1
                        while len(bdot) < len(temperatures):
                            bdot.extend([float(item) for item in dblist[line].split()])
                            line = line+1

                # Read the neutral species activity coefficients
                if 'co2' in dblist[line]:
                    if 'co2' not in neutral_species:
                        neutral_species['co2'] = {}

                    key = dblist[line].split()[1].strip()
                    line = line+1
                    vals = []
                    while not dblist[line].split()[0].isalpha():
                        vals.extend([float(item) for item in dblist[line].split()])
                        line = line+1

                    neutral_species['co2'][key] = vals
                    line = line-1

                if 'h2o' in dblist[line]:
                    if 'h2o' not in neutral_species:
                        neutral_species['h2o'] = {}

                    key = dblist[line].split()[1].strip()
                    line = line+1
                    vals = []
                    while len(vals) < len(temperatures):
                        vals.extend([float(item) for item in dblist[line].split()])
                        line = line+1

                    neutral_species['h2o'][key] = vals
                    line = line-1

                line = line+1

        # Read the elements
        if dblist[line].strip().startswith('elements') and dblist[line+1].strip().startswith('+--'):
            line = line+2

            while not dblist[line].strip().startswith('+--'):
                if dblist[line].split():
                    element, molecular_weight = dblist[line].split()
                    elements[element] = {}
                    elements[element]['molecular weight'] = float(molecular_weight)

                line = line+1

        # Read the basis species
        if dblist[line].strip().startswith('basis species') and dblist[line+1].strip().startswith('+--'):
            line = line+2

            # This section ends when it finds a line like
            # +--------
            # Some text
            # +--------
            parsing_basis = True
            while parsing_basis:
                species = dblist[line].split()[0]
                basis_species[species] = {}
                basis_species[species]['elements'] = {}
                line = line+1

                while not dblist[line].strip().startswith('+--'):
                    # Read molar weight
                    if 'mol.wt.' in dblist[line]:
                        basis_species[species]['molecular weight'] = float(dblist[line].split()[3])

                     # Read radius
                    if 'DHazero' in dblist[line]:
                        basis_species[species]['radius'] = float(dblist[line].split()[3])

                     # Read charge
                    if 'charge' in dblist[line]:
                        basis_species[species]['charge'] = float(dblist[line].split()[2])

                    # Read elements
                    if 'element(s)' in dblist[line]:
                        num_elements = int(dblist[line].split()[0])
                        while len(basis_species[species]['elements']) < num_elements:
                            line = line+1
                            data = dblist[line].split()
                            for i in range(0, len(data), 2):
                                basis_species[species]['elements'][data[i+1]] = float(data[i])

                    line = line+1

                if dblist[line].strip().startswith('+--') and dblist[line+2].strip().startswith('+--'):
                    parsing_basis = False

                line = line+1

        # Read the auxiliary species (redox couples)
        if dblist[line].strip().startswith('auxiliary basis species') and dblist[line+1].strip().startswith('+--'):
            line = line+2

            # This section ends when it finds a line like
            # +--------
            # Some text
            # +--------
            parsing_auxiliary = True
            while parsing_auxiliary:
                species = dblist[line].split()[0]
                redox_couples[species] = {}
                redox_couples[species]['elements'] = {}
                redox_couples[species]['species'] = {}
                line = line+1

                while not dblist[line].strip().startswith('+--'):
                    # Read molecular weight
                    if 'mol.wt.' in dblist[line]:
                        redox_couples[species]['molecular weight'] = float(dblist[line].split()[3])

                     # Read radius
                    if 'DHazero' in dblist[line]:
                        redox_couples[species]['radius'] = float(dblist[line].split()[3])

                     # Read charge
                    if 'charge' in dblist[line]:
                        redox_couples[species]['charge'] = float(dblist[line].split()[2])

                    # Read elements
                    if 'element' in dblist[line]:
                        num_elements = int(dblist[line].split()[0])
                        while len(redox_couples[species]['elements']) < num_elements:
                            line = line+1
                            data = dblist[line].split()
                            for i in range(0, len(data), 2):
                                redox_couples[species]['elements'][data[i+1]] = float(data[i])

                    # Species in redox couples
                    # (note: EQ3/6 includes species in this list, so we remove it)
                    if 'species' in dblist[line]:
                        num_species = int(dblist[line].split()[0])
                        while len(redox_couples[species]['species']) < num_species-1:
                            line = line+1
                            data = dblist[line].split()
                            for i in range(0, len(data), 2):
                                if data[i+1] != species:
                                    redox_couples[species]['species'][data[i+1]] = float(data[i])

                    # Equilibrium constant values
                    if 'logK grid' in dblist[line]:
                        line = line+1
                        vals = []
                        while len(vals) < len(temperatures):
                            vals.extend(dblist[line].split())
                            line = line+1

                    vals, note = fillValues(vals)
                    redox_couples[species]['logk'] = vals
                    if note:
                         redox_couples[species]['note'] = note

                    line = line+1

                if dblist[line].strip().startswith('+--') and dblist[line+2].strip().startswith('+--'):
                    parsing_auxiliary = False

                line = line+1

        # Read the aqueous species
        if dblist[line].strip().startswith('aqueous species') and dblist[line+1].strip().startswith('+--'):
            line = line+2

            # This section ends when it finds a line like
            # +--------
            # Some text
            # +--------
            parsing_secondary = True
            while parsing_secondary:
                species = dblist[line].split()[0]
                secondary_species[species] = {}
                secondary_species[species]['elements'] = {}
                secondary_species[species]['species'] = {}
                line = line+1

                while not dblist[line].strip().startswith('+--'):
                    # Read molecular weight
                    if 'mol.wt.' in dblist[line]:
                        secondary_species[species]['molecular weight'] = float(dblist[line].split()[3])

                     # Read radius
                    if 'DHazero' in dblist[line]:
                        secondary_species[species]['radius'] = float(dblist[line].split()[3])

                     # Read charge
                    if 'charge' in dblist[line]:
                        secondary_species[species]['charge'] = float(dblist[line].split()[2])

                    # Read elements
                    if 'element' in dblist[line]:
                        num_elements = int(dblist[line].split()[0])
                        while len(secondary_species[species]['elements']) < num_elements:
                            line = line+1
                            data = dblist[line].split()
                            for i in range(0, len(data), 2):
                                secondary_species[species]['elements'][data[i+1]] = float(data[i])

                    # Basis species in aqueous species
                    # (note: EQ3/6 includes aqueous species in this list, so we remove it)
                    if 'species' in dblist[line]:
                        num_species = int(dblist[line].split()[0])
                        while len(secondary_species[species]['species']) < num_species-1:
                            line = line+1
                            data = dblist[line].split()
                            for i in range(0, len(data), 2):
                                if data[i+1] != species:
                                    secondary_species[species]['species'][data[i+1]] = float(data[i])

                    # Equilibrium constant values
                    if 'logK grid' in dblist[line]:
                        line = line+1
                        vals = []
                        while len(vals) < len(temperatures):
                            vals.extend(dblist[line].split())
                            line = line+1

                    vals, note = fillValues(vals)
                    secondary_species[species]['logk'] = vals
                    if note:
                         secondary_species[species]['note'] = note
                    line = line+1

                if dblist[line].strip().startswith('+--') and dblist[line+2].strip().startswith('+--'):
                    parsing_secondary = False

                line = line+1

        # Read the mineral (solid) species
        if dblist[line].strip().startswith('solids') and dblist[line+1].strip().startswith('+--'):
            line = line+2

            # This section ends when it finds a line like
            # +--------
            # Some text
            # +--------
            parsing_mineral = True
            while parsing_mineral:
                species = dblist[line].split()[0]
                mineral_species[species] = {}
                mineral_species[species]['elements'] = {}
                mineral_species[species]['species'] = {}
                line = line+1

                while not dblist[line].strip().startswith('+--'):
                    # Read molecular weight
                    if 'mol.wt.' in dblist[line]:
                        mineral_species[species]['molecular weight'] = float(dblist[line].split()[3])

                     # Read molar volume
                    if 'V0PrTr' in dblist[line]:
                        mineral_species[species]['molar volume'] = float(dblist[line].split()[2])

                    # Read elements
                    if 'element' in dblist[line]:
                        num_elements = int(dblist[line].split()[0])
                        while len(mineral_species[species]['elements']) < num_elements:
                            line = line+1
                            data = dblist[line].split()
                            for i in range(0, len(data), 2):
                                mineral_species[species]['elements'][data[i+1]] = float(data[i])

                    # Basis species in solid species
                    # (note: EQ3/6 includes solid species in this list, so we remove it)
                    if 'species' in dblist[line]:
                        num_species = int(dblist[line].split()[0])
                        while len(mineral_species[species]['species']) < num_species-1:
                            line = line+1
                            data = dblist[line].split()
                            for i in range(0, len(data), 2):
                                if data[i+1] != species:
                                    mineral_species[species]['species'][data[i+1]] = float(data[i])

                    # Equilibrium constant values
                    if 'logK grid' in dblist[line]:
                        line = line+1
                        vals = []
                        while len(vals) < len(temperatures):
                            vals.extend(dblist[line].split())
                            line = line+1

                    vals, note = fillValues(vals)
                    mineral_species[species]['logk'] = vals
                    if note:
                         mineral_species[species]['note'] = note
                    line = line+1

                if dblist[line].strip().startswith('+--') and dblist[line+2].strip().startswith('+--'):
                    parsing_mineral = False

                line = line+1

        # Read the gas species
        if dblist[line].strip().startswith('gases') and dblist[line+1].strip().startswith('+--'):
            line = line+2

            # This section ends when it finds a line like
            # +--------
            # Some text
            # +--------
            parsing_gas = True
            while parsing_gas:
                species = dblist[line].split()[0]
                gas_species[species] = {}
                gas_species[species]['elements'] = {}
                gas_species[species]['species'] = {}
                line = line+1

                while not dblist[line].strip().startswith('+--'):
                    # Read molecular weight
                    if 'mol.wt.' in dblist[line]:
                        gas_species[species]['molecular weight'] = float(dblist[line].split()[3])

                    # Read elements
                    if 'element' in dblist[line]:
                        num_elements = int(dblist[line].split()[0])
                        while len(gas_species[species]['elements']) < num_elements:
                            line = line+1
                            data = dblist[line].split()
                            for i in range(0, len(data), 2):
                                gas_species[species]['elements'][data[i+1]] = float(data[i])

                    # Basis species in gas species
                    # (note: EQ3/6 includes gas species in this list, so we remove it)
                    if 'species in' in dblist[line]:
                        num_species = int(dblist[line].split()[0])
                        while len(gas_species[species]['species']) < num_species-1:
                            line = line+1
                            data = dblist[line].split()
                            for i in range(0, len(data), 2):
                                if data[i+1] != species or not data[i].startswith('-1'):
                                    gas_species[species]['species'][data[i+1]] = float(data[i])

                    # Equilibrium constant values
                    if 'logK grid' in dblist[line]:
                        line = line+1
                        vals = []
                        while len(vals) < len(temperatures):
                            vals.extend(dblist[line].split())
                            line = line+1

                    vals, note = fillValues(vals)
                    gas_species[species]['logk'] = vals
                    if note:
                         gas_species[species]['note'] = note
                    line = line+1

                if dblist[line].strip().startswith('+--') and dblist[line+2].strip().startswith('+--'):
                    parsing_gas = False

                line = line+1

        line = line+1

    # After parsing all of the data, save it in a ThermodB class
    db = ThermoDB()
    db.format = 'eq36'
    db.header = dataset
    db.activity_model = activity_model
    if activity_model == 'debye-huckel':
        db.adh = adh
        db.bdh = bdh
        db.bdot = bdot
    db.fugacity_model = fugacity_model
    db.logk_model = logk_model
    db.logk_model_eqn = logk_model_eqn
    db.temperatures = temperatures
    db.pressures = pressures
    db.neutral_species = neutral_species
    db.elements = elements
    db.basis_species = basis_species
    db.secondary_species = secondary_species
    db.mineral_species = mineral_species
    db.gas_species = gas_species
    db.redox_couples = redox_couples
    db.oxides = oxides

    return db
