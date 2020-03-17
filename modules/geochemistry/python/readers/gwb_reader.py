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

def readDatabase(dblist):
    """
    Read a Geochemists Workbench database file
    """

    activity_model = None
    fugacity_model = None
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

    line = 0

    while line < len(dblist):

        if 'activity model' in dblist[line]:
            activity_model = dblist[line].split(':')[1].strip()

        if 'fugacity model' in dblist[line]:
            fugacity_model = dblist[line].split(':')[1].strip()

        if 'temperatures' in dblist[line]:
            line = line+1
            while dblist[line].strip()[0].isnumeric():
                temperatures.extend(dblist[line].split())
                line = line+1

        if 'pressures' in dblist[line]:
            line = line+1
            while len(pressures) < len(temperatures):
                pressures.extend(dblist[line].split())
                line = line+1

        if activity_model == 'debye-huckel':
            # Read Debye-Huckel activity coefficients
            if 'debye huckel a' in dblist[line]:
                line = line+1
                while len(adh) < len(temperatures):
                    adh.extend(dblist[line].split())
                    line = line+1

            if 'debye huckel b' in dblist[line]:
                line = line+1
                while len(bdh) < len(temperatures):
                    bdh.extend(dblist[line].split())
                    line = line+1

            if 'bdot' in dblist[line]:
                line = line+1
                while len(bdot) < len(temperatures):
                    bdot.extend(dblist[line].split())
                    line = line+1

        # Read the neutral species activity coefficients
        labels = ['a', 'b', 'c', 'd']
        if 'co2' in dblist[line]:
            if 'co2' not in neutral_species:
                neutral_species['co2'] = {}

            key = labels[int(dblist[line].split('*')[-1].split()[2])-1]
            line = line+1
            vals = []
            while len(vals) < len(temperatures):
                vals.extend(dblist[line].split())
                line = line+1

            neutral_species['co2'][key] = vals
            line = line-1

        if 'h2o' in dblist[line]:
            if 'h2o' not in neutral_species:
                neutral_species['h2o'] = {}

            key = labels[int(dblist[line].split('*')[-1].split()[2])-1]
            line = line+1
            vals = []
            while len(vals) < len(temperatures):
                vals.extend(dblist[line].split())
                line = line+1

            neutral_species['h2o'][key] = vals
            line = line-1

        if 'elements\n' in dblist[line]:
            # Read the elements
            line = line+1

            reading_elements = True
            while reading_elements:
                if dblist[line].split():
                    elemlist = dblist[line].replace('(','').replace(')','').split()
                    elements[elemlist[1]] = {}
                    elements[elemlist[1]]['name'] = elemlist[0]
                    elements[elemlist[1]]['molecular weight'] = elemlist[4]
                line = line+1

                if '-end-' in dblist[line]:
                    reading_elements = False
                    line = line-1

        if 'basis species\n' in dblist[line]:
            # Read the basis species
            line = line+1
            reading_basis = True
            while reading_basis:
                if dblist[line].strip():
                    # Species name
                    species = dblist[line].strip()
                    basis_species[species] = {}
                    basis_species[species]['elements'] = {}
                    line = line+1

                    # Charge, ionic radius and molecular weight
                    if 'charge' in dblist[line]:
                        data = dblist[line].replace('=', ' ').split()
                        basis_species[species]['charge'] = data[1]
                        basis_species[species]['radius'] = data[4]
                        basis_species[species]['molecular weight'] = data[8]
                        line = line+1

                    # Elements in basis species
                    if 'elements' in dblist[line]:
                        num_elements = int(dblist[line].split()[0])
                        while len(basis_species[species]['elements']) < num_elements:
                            line = line+1
                            data = dblist[line].split()
                            for i in range(0, len(data), 2):
                                basis_species[species]['elements'][data[i+1]] = data[i]


                line = line+1

                if '-end-' in dblist[line]:
                    reading_basis = False
                    line = line-1

        if 'redox couples\n' in dblist[line]:
            # Read the redox couples
            line = line+1
            reading_redox = True
            while reading_redox:
                if dblist[line].strip():
                    # Couple name
                    couple = dblist[line].strip()
                    redox_couples[couple] = {}
                    redox_couples[couple]['species'] = {}
                    line = line+1

                    # Charge, ionic radius and molecular weight
                    if 'charge' in dblist[line]:
                        data = dblist[line].replace('=', ' ').split()
                        redox_couples[couple]['charge'] = data[1]
                        redox_couples[couple]['radius'] = data[4]
                        redox_couples[couple]['molecular weight'] = data[8]
                        line = line+1

                    # Species in redox couples
                    if 'species' in dblist[line]:
                        num_species = int(dblist[line].split()[0])
                        while len(redox_couples[couple]['species']) < num_species:
                            line = line+1
                            data = dblist[line].split()
                            for i in range(0, len(data), 2):
                                redox_couples[couple]['species'][data[i+1]] = data[i]

                        line = line+1

                    # Equilibrium constant values
                    vals = []
                    while len(vals) < len(temperatures):
                        vals.extend(dblist[line].split())
                        line = line+1
                    redox_couples[couple]['logk'] = vals

                line = line+1

                if '-end-' in dblist[line]:
                    reading_redox = False
                    line = line-1

        if 'aqueous species\n' in dblist[line]:
            # Read the aqueous species (secondary species)
            line = line+1
            reading_secondary = True
            while reading_secondary:
                if dblist[line].strip():
                    # Couple name
                    species = dblist[line].strip()
                    secondary_species[species] = {}
                    secondary_species[species]['species'] = {}
                    line = line+1

                    # Charge, ionic radius and molecular weight
                    if 'charge' in dblist[line]:
                        data = dblist[line].replace('=', ' ').split()
                        secondary_species[species]['charge'] = data[1]
                        secondary_species[species]['radius'] = data[4]
                        secondary_species[species]['molecular weight'] = data[8]
                        line = line+1

                    # Species in secondary reactions
                    if 'species' in dblist[line]:
                        num_species = int(dblist[line].split()[0])
                        while len(secondary_species[species]['species']) < num_species:
                            line = line+1
                            data = dblist[line].split()
                            for i in range(0, len(data), 2):
                                secondary_species[species]['species'][data[i+1]] = data[i]

                        line = line+1

                    # Equilibrium constant values
                    vals = []
                    while len(vals) < len(temperatures):
                        vals.extend(dblist[line].split())
                        line = line+1
                    secondary_species[species]['logk'] = vals

                line = line+1

                if '-end-' in dblist[line]:
                    reading_secondary = False
                    line = line-1

        if 'free electron\n' in dblist[line]:
            # Read the free electron data
            line = line+1
            reading_freeelectron = True
            while reading_freeelectron:
                if dblist[line].strip():
                    # Species name
                    species = dblist[line].strip()
                    free_electron[species] = {}
                    free_electron[species]['species'] = {}
                    line = line+1

                    # Charge, ionic radius and molecular weight
                    if 'charge' in dblist[line]:
                        data = dblist[line].replace('=', ' ').split()
                        free_electron[species]['charge'] = data[1]
                        free_electron[species]['radius'] = data[4]
                        free_electron[species]['molecular weight'] = data[8]
                        line = line+1

                    # Species in free electron
                    if 'species' in dblist[line]:
                        num_species = int(dblist[line].split()[0])
                        while len(free_electron[species]['species']) < num_species:
                            line = line+1
                            data = dblist[line].split()
                            for i in range(0, len(data), 2):
                                free_electron[species]['species'][data[i+1]] = data[i]

                        line = line+1

                    # Equilibrium constant values
                    vals = []
                    while len(vals) < len(temperatures):
                        vals.extend(dblist[line].split())
                        line = line+1
                    free_electron[species]['logk'] = vals

                line = line+1

                if '-end-' in dblist[line]:
                    reading_freeelectron = False
                    line = line-1

        if 'minerals\n' in dblist[line]:
            # Read the minerals data
            line = line+1
            reading_mineral_species = True
            while reading_mineral_species:
                if dblist[line].strip():
                    # Species name
                    species = dblist[line].strip().split()[0]
                    mineral_species[species] = {}
                    mineral_species[species]['species'] = {}
                    line = line+1

                    if 'formula' in dblist[line]:
                        line = line+1

                    # molar volume and molecular weight
                    if 'mole' in dblist[line]:
                        data = dblist[line].replace('=', ' ').split()
                        mineral_species[species]['molar volume'] = data[2]
                        mineral_species[species]['molecular weight'] = data[6]
                        line = line+1

                    # Species in minerals
                    if 'species' in dblist[line]:
                        num_species = int(dblist[line].split()[0])
                        while len(mineral_species[species]['species']) < num_species:
                            line = line+1
                            data = dblist[line].split()
                            for i in range(0, len(data), 2):
                                mineral_species[species]['species'][data[i+1]] = data[i]

                        line = line+1

                    # Equilibrium constant values
                    vals = []
                    while len(vals) < len(temperatures):
                        vals.extend(dblist[line].split())
                        line = line+1
                    mineral_species[species]['logk'] = vals

                line = line+1

                if '-end-' in dblist[line]:
                    reading_mineral_species = False
                    line = line-1

        if 'gases\n' in dblist[line]:
            # Read the gas data
            line = line+1
            reading_gas_species = True
            while reading_gas_species:
                if dblist[line].strip():
                    # Species name
                    species = dblist[line].strip().split()[0]
                    gas_species[species] = {}
                    gas_species[species]['species'] = {}
                    line = line+1

                    # molecular weight
                    if 'mole' in dblist[line]:
                        data = dblist[line].replace('=', ' ').split()
                        gas_species[species]['molecular weight'] = data[2]
                        line = line+1

                    # Spycher-Reed fugacity model coefficients
                    if 'chi' in dblist[line]:
                        gas_species[species]['chi'] = dblist[line].split()[1:]
                        line = line+1

                    # Tsonopoulos fugacity model coefficients
                    if 'Pcrit' in dblist[line]:
                        data = dblist[line].replace('=', ' ').split()
                        gas_species[species][data[0]] = data[1]
                        gas_species[species][data[3]] = data[4]
                        gas_species[species][data[6]] = data[7]

                        line = line+1

                    # Species in gas_species
                    if 'species' in dblist[line]:
                        num_species = int(dblist[line].split()[0])
                        while len(gas_species[species]['species']) < num_species:
                            line = line+1
                            data = dblist[line].split()
                            for i in range(0, len(data), 2):
                                gas_species[species]['species'][data[i+1]] = data[i]

                        line = line+1

                    # Equilibrium constant values
                    vals = []
                    while len(vals) < len(temperatures):
                        vals.extend(dblist[line].split())
                        line = line+1
                    gas_species[species]['logk'] = vals

                line = line+1

                if '-end-' in dblist[line]:
                    reading_gas_species = False
                    line = line-1

        if 'oxides\n' in dblist[line]:
            # Read the oxide data
            line = line+1
            reading_oxides = True
            while reading_oxides:
                if dblist[line].strip():
                    # Species name
                    species = dblist[line].strip().split()[0]
                    oxides[species] = {}
                    oxides[species]['species'] = {}
                    line = line+1

                    # molecular weight
                    if 'mole' in dblist[line]:
                        data = dblist[line].replace('=', ' ').split()
                        oxides[species]['molecular weight'] = data[2]
                        line = line+1

                    # Species in gas_species
                    if 'species' in dblist[line]:
                        num_species = int(dblist[line].split()[0])
                        while len(oxides[species]['species']) < num_species:
                            line = line+1
                            data = dblist[line].split()
                            for i in range(0, len(data), 2):
                                oxides[species]['species'][data[i+1]] = data[i]

                        line = line+1

                line = line+1

                if '-end-' in dblist[line]:
                    reading_oxides = False
                    line = line-1

        line = line+1

    # After parsing all of the data, save it in a ThermodB class
    db = ThermoDB()
    db.format = 'gwb'
    db.activity_model = activity_model
    if activity_model == 'debye-huckel':
        db.adh = adh
        db.bdh = bdh
        db.bdot = bdot
    db.fugacity_model = fugacity_model
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
