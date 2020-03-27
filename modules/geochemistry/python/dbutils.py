#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

def secondarySpeciesContainingBasis(db, sec_species_type, basis_species):
    """
    Searches the database and returns all secondary species of given type
    containing at least one of the basis species
    """
    assert sec_species_type in db, "Specified secondary species type not in database"

    return [key for (key, value) in db[sec_species_type].items() if any(b in value['species'] for b in basis_species)]

def printSpeciesInfo(db, species):
    """
    Find the given species and return all information related to it
    """
    while True:
        # Check basis species
        if db['basis species']:
            if species in db['basis species']:
                type, result = 'basis species', db['basis species'][species]
                break

        # Check secondary species
        if db['secondary species']:
            if species in db['secondary species']:
                type, result = 'secondary species', db['secondary species'][species]
                break

        # Check redox couples
        if db['redox couples']:
            if species in db['redox couples']:
                type, result = 'redox couple', db['redox couples'][species]
                break

        # Check gas species
        if db['gas species']:
            if species in db['gas species']:
                type, result = 'gas species', db['gas species'][species]
                break

        # Check minerals
        if db['mineral species']:
            if species in db['mineral species']:
                type, result = 'mineral species', db['mineral species'][species]
                break

        # If we get here, species is not is database
        print(species, "not in database")
        return

    # Now print out the information
    indent = '  '
    print(species + ':')
    print(indent + 'type:', type)
    for (key, values) in result.items():
        print(indent + key + ": ", values)

    return

def printReaction(db, sec_species_type, sec_species):
    """
    Print a readable version of the general reaction
    """

    assert sec_species in db[sec_species_type], "Secondary species not in database"

    reaction = sec_species + ' = '
    for species, weight in db[sec_species_type][sec_species]['species'].items():
        sgn = ' '
        if float(weight) > 0:
            sgn = ' + '

        reaction += sgn + weight + ' ' + species

    # Remove sgn after = if it is a +
    reaction = reaction.split()
    if reaction[2] == '+':
        reaction.pop(2)
    print(' '.join(reaction))
    return

def printEquilibriumReaction(db, sec_species):
    """
    Print a readable version of the aqueous equilibrium reaction
    """

    return printReaction(db, 'secondary species', sec_species)

def printMineralReaction(db, sec_species):
    """
    Print a readable version of the mineral reaction
    """

    return printReaction(db, 'mineral species', sec_species)

def printGasReaction(db, sec_species):
    """
    Print a readable version of the gas reaction
    """

    return printReaction(db, 'gas species', sec_species)

def printRedoxReaction(db, sec_species):
    """
    Print a readable version of the redox reaction
    """

    return printReaction(db, 'redox couples', sec_species)
