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
    Read a Geochemists Workbench database file
    """

    activity_model = None
    fugacity_model = None
    logk_model = 'fourth-order'
    logk_model_eqn = 'a0 + a1 T + a2 T^2 + a3 T^3 + a4 T^4'
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
    sorbing_minerals = {}
    surface_species = {}

    missing_value = '500.0000'
    line = 0

    # Helper function for interpolating missing array values
    # (to reduce number of paramters required during use)
    def fillValues(vals):
        return reader_utils.fillMissingValues(temperatures, vals, logk_model, missing_value)

    while line < len(dblist):

        if 'activity model' in dblist[line]:
            activity_model = dblist[line].split(':')[1].strip()

        if 'fugacity model' in dblist[line]:
            fugacity_model = dblist[line].split(':')[1].strip()

        if 'temperatures' in dblist[line]:
            line = line+1
            while dblist[line].strip()[0].isnumeric():
                temperatures.extend([float(item) for item in dblist[line].split()])
                line = line+1

        if 'pressures' in dblist[line]:
            line = line+1
            while len(pressures) < len(temperatures):
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
        labels = ['a', 'b', 'c', 'd']
        if 'c co2' in dblist[line]:
            if 'co2' not in neutral_species:
                neutral_species['co2'] = {}

            key = labels[int(dblist[line].split('*')[-1].split()[2])-1]
            line = line+1
            vals = []
            while len(vals) < len(temperatures):
                vals.extend([float(item) for item in dblist[line].split()])
                line = line+1

            vals, note = fillValues(vals)
            neutral_species['co2'][key] = vals
            if note:
                 neutral_species['co2']['note'] = note
            line = line-1

        if 'c h2o' in dblist[line]:
            if 'h2o' not in neutral_species:
                neutral_species['h2o'] = {}

            key = labels[int(dblist[line].split('*')[-1].split()[2])-1]
            line = line+1
            vals = []
            while len(vals) < len(temperatures):
                vals.extend(dblist[line].split())
                line = line+1

            vals, note = fillValues(vals)
            neutral_species['h2o'][key] = vals
            if note:
                 neutral_species['h2o']['note'] = note
            line = line-1

        if 'elements\n' in dblist[line] and len(dblist[line].split()) == 2:
            # Read the elements
            line = line+1

            reading_elements = True
            while reading_elements:
                if dblist[line].split():
                    elemlist = dblist[line].replace('(','').replace(')','').split()
                    if len(elemlist) != 6 or elemlist[2] != "mole" or elemlist[3] != "wt.=" or elemlist[5] != "g":
                        raise ValueError("Element line incorrectly specified.  Offending line is: " + dblist[line])
                    elements[elemlist[1]] = {}
                    elements[elemlist[1]]['name'] = elemlist[0]
                    elements[elemlist[1]]['molecular weight'] = float(elemlist[4])
                line = line+1

                if '-end-' in dblist[line]:
                    reading_elements = False
                    line = line-1

        if 'basis species\n' in dblist[line] and len(dblist[line].split()) == 3:
            # Read the basis species
            line = line+1
            reading_basis = True
            while reading_basis:
                if dblist[line].strip():
                    # Species name
                    species = dblist[line].strip()
                    basis_species[species] = {}
                    basis_species[species]['elements'] = {}
                    basis_species[species]['radius'] = 0 # default for sorption sites that are counted as basis species
                    line = line+1

                    # Charge, ionic radius and molecular weight
                    if 'charge' in dblist[line]:
                        data = dblist[line][dblist[line].index("charge") + 6:].split("=")[1]
                        basis_species[species]['charge'] = float(data.split()[0])
                        if len(data.split()) <= 1:
                            line = line+1
                    if 'ion size' in dblist[line]:
                        data = dblist[line][dblist[line].index("ion size") + 8:].split("=")[1]
                        if len(data.split()) <= 1 or data.split()[1] != "A":
                            raise ValueError("ion size must be measured in Angstroms.  Offending line is: " + dblist[line])
                        basis_species[species]['radius'] = float(data.split()[0])
                        if len(data.split()) <= 2:
                            line = line + 1
                    if 'mole wt.' in dblist[line]:
                        data = dblist[line][dblist[line].index("mole wt.") + 8:].split("=")[1]
                        if len(data.split()) <= 1 or data.split()[1] != "g":
                            raise ValueError("molecular weight must be measured in grams.  Offending line is: " + dblist[line])
                        basis_species[species]['molecular weight'] = float(data.split()[0])
                        if len(data.split()) <= 2:
                            line = line + 1

                    # Elements in basis species
                    if 'elements in species' in dblist[line]:
                        num_elements = int(dblist[line].split()[0])
                        while len(basis_species[species]['elements']) < num_elements:
                            line = line+1
                            data = dblist[line].split()
                            for i in range(0, len(data), 2):
                                basis_species[species]['elements'][data[i+1]] = float(data[i])

                line = line+1

                if '-end-' in dblist[line]:
                    reading_basis = False
                    line = line-1

        if 'redox couples\n' in dblist[line] and len(dblist[line].split()) == 3:
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
                        data = dblist[line][dblist[line].index("charge") + 6:].split("=")[1]
                        redox_couples[couple]['charge'] = float(data.split()[0])
                        if len(data.split()) <= 1:
                            line = line+1
                    if 'ion size' in dblist[line]:
                        data = dblist[line][dblist[line].index("ion size") + 8:].split("=")[1]
                        if len(data.split()) <= 1 or data.split()[1] != "A":
                            raise ValueError("ion size must be measured in Angstroms.  Offending line is: " + dblist[line])
                        redox_couples[couple]['radius'] = float(data.split()[0])
                        if len(data.split()) <= 2:
                            line = line + 1
                    if 'mole wt.' in dblist[line]:
                        data = dblist[line][dblist[line].index("mole wt.") + 8:].split("=")[1]
                        if len(data.split()) <= 1 or data.split()[1] != "g":
                            raise ValueError("molecular weight must be measured in grams.  Offending line is: " + dblist[line])
                        redox_couples[couple]['molecular weight'] = float(data.split()[0])
                        if len(data.split()) <= 2:
                            line = line + 1

                    # Species in redox couples
                    if 'species in reaction' in dblist[line]:
                        num_species = int(dblist[line].split()[0])
                        while len(redox_couples[couple]['species']) < num_species:
                            line = line+1
                            data = dblist[line].split()
                            for i in range(0, len(data), 2):
                                redox_couples[couple]['species'][data[i+1]] = float(data[i])

                        line = line+1

                    # Equilibrium constant values
                    vals = []
                    while len(vals) < len(temperatures):
                        vals.extend([float(item) for item in dblist[line].split()])
                        line = line+1

                    vals, note = fillValues(vals)
                    redox_couples[couple]['logk'] = vals
                    if note:
                         redox_couples[couple]['note'] = note

                line = line+1

                if '-end-' in dblist[line]:
                    reading_redox = False
                    line = line-1

        if 'aqueous species\n' in dblist[line] and len(dblist[line].split()) == 3:
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
                        data = dblist[line][dblist[line].index("charge") + 6:].split("=")[1]
                        secondary_species[species]['charge'] = float(data.split()[0])
                        if len(data.split()) <= 1:
                            line = line+1
                    if 'ion size' in dblist[line]:
                        data = dblist[line][dblist[line].index("ion size") + 8:].split("=")[1]
                        if len(data.split()) <= 1 or data.split()[1] != "A":
                            raise ValueError("ion size must be measured in Angstroms.  Offending line is: " + dblist[line])
                        secondary_species[species]['radius'] = float(data.split()[0])
                        if len(data.split()) <= 2:
                            line = line + 1
                    if 'mole wt.' in dblist[line]:
                        data = dblist[line][dblist[line].index("mole wt.") + 8:].split("=")[1]
                        if len(data.split()) <= 1 or data.split()[1] != "g":
                            raise ValueError("molecular weight must be measured in grams.  Offending line is: " + dblist[line])
                        secondary_species[species]['molecular weight'] = float(data.split()[0])
                        if len(data.split()) <= 2:
                            line = line + 1

                    # Species in secondary reactions
                    if 'species' in dblist[line]:
                        num_species = int(dblist[line].split()[0])
                        while len(secondary_species[species]['species']) < num_species:
                            line = line+1
                            data = dblist[line].split()
                            for i in range(0, len(data), 2):
                                secondary_species[species]['species'][data[i+1]] = float(data[i])

                        line = line+1

                    # Equilibrium constant values
                    vals = []
                    while len(vals) < len(temperatures):
                        vals.extend(dblist[line].split())
                        line = line+1

                    vals, note = fillValues(vals)
                    secondary_species[species]['logk'] = vals
                    if note:
                         secondary_species[species]['note'] = note

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
                        data = dblist[line][dblist[line].index("charge") + 6:].split("=")[1]
                        free_electron[species]['charge'] = float(data.split()[0])
                        if len(data.split()) <= 1:
                            line = line+1
                    if 'ion size' in dblist[line]:
                        data = dblist[line][dblist[line].index("ion size") + 8:].split("=")[1]
                        if len(data.split()) <= 1 or data.split()[1] != "A":
                            raise ValueError("ion size must be measured in Angstroms.  Offending line is: " + dblist[line])
                        free_electron[species]['radius'] = float(data.split()[0])
                        if len(data.split()) <= 2:
                            line = line + 1
                    if 'mole wt.' in dblist[line]:
                        data = dblist[line][dblist[line].index("mole wt.") + 8:].split("=")[1]
                        if len(data.split()) <= 1 or data.split()[1] != "g":
                            raise ValueError("molecular weight must be measured in grams.  Offending line is: " + dblist[line])
                        free_electron[species]['molecular weight'] = float(data.split()[0])
                        if len(data.split()) <= 2:
                            line = line + 1

                    # Species in free electron
                    if 'species' in dblist[line]:
                        num_species = int(dblist[line].split()[0])
                        while len(free_electron[species]['species']) < num_species:
                            line = line+1
                            data = dblist[line].split()
                            for i in range(0, len(data), 2):
                                free_electron[species]['species'][data[i+1]] = float(data[i])

                        line = line+1

                    # Equilibrium constant values
                    vals = []
                    while len(vals) < len(temperatures):
                        vals.extend(dblist[line].split())
                        line = line+1

                    vals, note = fillValues(vals)
                    free_electron[species]['logk'] = vals
                    if note:
                         free_electron[species]['note'] = note

                line = line+1

                if '-end-' in dblist[line]:
                    reading_freeelectron = False
                    line = line-1

        if 'sorbing minerals\n' in dblist[line]:
            # Read sorbing minerals
            line = line+1
            reading_sorbing_mineral_species = True
            while reading_sorbing_mineral_species:
                if dblist[line].strip():
                    # Mineral name
                    species = dblist[line].strip().split()[0]
                    sorbing_minerals[species] = {}
                    sorbing_minerals[species]['sorbing sites'] = {}
                    line = line+1

                    # specific surface area
                    if 'surface area' in dblist[line]:
                        data = dblist[line].split("surface area")[1].split("=")[1]
                        if len(data.split()) <= 1 or data.split()[1] != "m2/g":
                            raise ValueError("sorbing mineral surface area must be specified in m2/g.  Offending line is: " + dblist[line])
                        sorbing_minerals[species]['surface area'] = float(data.split()[0])
                        line = line+1

                    if 'sorption sites' in dblist[line] and len(dblist[line].split()) == 3:
                        num_sites = int(dblist[line].split()[0])
                        while len(sorbing_minerals[species]['sorbing sites']) < num_sites:
                            line += 1
                            data = dblist[line].split()
                            if len(data) != 6 and data[1] != "site" and data[2] != "density=" and data[4] != "mol/mol" and data[5] != "mineral":
                                raise ValueError("sorbing minerals must define site density in mol/mol mineral.  Offending line is: " + dblist[line])
                            sorbing_minerals[species]['sorbing sites'][data[0]] = float(data[3]);

                        line = line+1

                line = line+1

                if '-end-' in dblist[line]:
                    reading_sorbing_mineral_species = False
                    line = line-1

        if 'minerals\n' in dblist[line] and not ('sorbing minerals' in dblist[line]) and len(dblist[line].split()) == 2:
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
                        if len(data) != 8 or data[0] != "mole" or data[1] != "vol." or data[3] != "cc" or data[4] != "mole" or data[5] != "wt." or data[7] != "g":
                            raise ValueError("molar volume must be measured in cc and molecular weight in grams.  Offending line is: " + dblist[line])
                        mineral_species[species]['molar volume'] = float(data[2])
                        mineral_species[species]['molecular weight'] = float(data[6])
                        line = line+1

                    # Species in minerals
                    if 'species' in dblist[line]:
                        num_species = int(dblist[line].split()[0])
                        while len(mineral_species[species]['species']) < num_species:
                            line = line+1
                            data = dblist[line].split()
                            for i in range(0, len(data), 2):
                                mineral_species[species]['species'][data[i+1]] = float(data[i])

                        line = line+1

                    # Equilibrium constant values
                    vals = []
                    while len(vals) < len(temperatures):
                        vals.extend(dblist[line].split())
                        line = line+1

                    vals, note = fillValues(vals)
                    mineral_species[species]['logk'] = vals
                    if note:
                         mineral_species[species]['note'] = note

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
                        if len(data) != 4 or data[0] != "mole" or data[1] != "wt." or data[3] != "g":
                            raise ValueError("molecular weight must be measured in grams.  Offending line is: " + dblist[line])
                        gas_species[species]['molecular weight'] = float(data[2])
                        line = line+1

                    # Spycher-Reed fugacity model coefficients
                    if 'chi' in dblist[line]:
                        gas_species[species]['chi'] = [float(item) for item in dblist[line].split()[1:]]
                        line = line+1

                    # Tsonopoulos fugacity model coefficients
                    if 'Pcrit' in dblist[line]:
                        data = dblist[line].replace('=', ' ').split()
                        if not (len(data) == 8 or len(data) == 12) or data[0] != "Pcrit" or data[2] != "bar" or data[3] != "Tcrit" or data[5] != "K" or data[6] != "omega":
                            raise ValueError("Tsonopoulos parameters must be measured in bar and Kelvin.  Offending line is: " + dblist[line])
                        gas_species[species][data[0]] = float(data[1])
                        gas_species[species][data[3]] = float(data[4])
                        gas_species[species][data[6]] = float(data[7])
                        if len(data) == 8:
                            gas_species[species]["a"] = 0.0
                            gas_species[species]["b"] = 0.0
                        else:
                            gas_species[species]["a"] = float(data[9])
                            gas_species[species]["b"] = float(data[11])

                        line = line+1

                    # Species in gas_species
                    if 'species' in dblist[line]:
                        num_species = int(dblist[line].split()[0])
                        while len(gas_species[species]['species']) < num_species:
                            line = line+1
                            data = dblist[line].split()
                            for i in range(0, len(data), 2):
                                gas_species[species]['species'][data[i+1]] = float(data[i])

                        line = line+1

                    # Equilibrium constant values
                    vals = []
                    while len(vals) < len(temperatures):
                        vals.extend(dblist[line].split())
                        line = line+1

                    vals, note = fillValues(vals)
                    gas_species[species]['logk'] = vals
                    if note:
                         gas_species[species]['note'] = note

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
                        if len(data) != 4 or data[0] != "mole" or data[1] != "wt." or data[3] != "g":
                            raise ValueError("Oxide molecular weight must be measured in grams.  Offending line is: " + dblist[line])
                        oxides[species]['molecular weight'] = float(data[2])
                        line = line+1

                    # Species in gas_species
                    if 'species' in dblist[line]:
                        num_species = int(dblist[line].split()[0])
                        while len(oxides[species]['species']) < num_species:
                            line = line+1
                            data = dblist[line].split()
                            for i in range(0, len(data), 2):
                                oxides[species]['species'][data[i+1]] = float(data[i])

                        line = line+1

                line = line+1

                if '-end-' in dblist[line]:
                    reading_oxides = False
                    line = line-1

        if 'surface species\n' in dblist[line]:
            # Read the surface species (those that sorb to a surface)
            line = line+1
            reading_ss = True
            while reading_ss:
                if dblist[line].strip():
                    # surface species name
                    species = dblist[line].strip()
                    surface_species[species] = {}
                    surface_species[species]['species'] = {}
                    line = line+1

                    # Charge and molecular weight
                    if 'charge' in dblist[line]:
                        data = dblist[line][dblist[line].index("charge") + 6:].split("=")[1]
                        surface_species[species]['charge'] = float(data.split()[0])
                        if len(data.split()) <= 1:
                            line = line+1
                    if 'mole wt.' in dblist[line]:
                        data = dblist[line][dblist[line].index("mole wt.") + 8:].split("=")[1]
                        if len(data.split()) <= 1 or data.split()[1] != "g":
                            raise ValueError("molecular weight must be measured in grams.  Offending line is " + dblist[line])
                        surface_species[species]['molecular weight'] = float(data.split()[0])
                        if len(data.split()) <= 2:
                            line = line + 1

                    # Species in reactions
                    if 'species' in dblist[line]:
                        num_species = int(dblist[line].split()[0])
                        while len(surface_species[species]['species']) < num_species:
                            line = line+1
                            data = dblist[line].split()
                            for i in range(0, len(data), 2):
                                surface_species[species]['species'][data[i+1]] = float(data[i])
                        line = line+1

                    # Equilibrium constant values
                    if 'log K' in dblist[line]:
                        data = dblist[line].split()
                        if len(data) != 5 and data[0] != "log" and data[1] != "K=" and data[3] != "dlogK/dT=":
                            raise ValueError("log K and dlogK/dT must be specified for surface species.  Offending line is " + dblist[line])
                        surface_species[species]["log K"] = float(data[2])
                        surface_species[species]["dlogK/dT"] = float(data[4])

                line = line+1

                if '-end-' in dblist[line]:
                    reading_ss = False
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
    db.logk_model = logk_model
    db.logk_model_eqn = logk_model_eqn
    db.temperatures = temperatures
    db.pressures = pressures
    db.neutral_species = neutral_species
    db.elements = elements
    db.basis_species = basis_species
    db.secondary_species = secondary_species
    db.free_electron = free_electron
    db.mineral_species = mineral_species
    db.sorbing_minerals = sorbing_minerals
    db.gas_species = gas_species
    db.redox_couples = redox_couples
    db.oxides = oxides
    db.surface_species = surface_species

    return db
