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
import argparse
import datetime
import json
# so we can find our libraries, no matter how we're called
findbin = os.path.dirname(os.path.realpath(sys.argv[0]))
sys.path.append(findbin)
from readers import gwb_reader, eq36_reader

def command_line_options():
    """
    Command-line options for the thermodynamic database reader and converter
    """
    parser = argparse.ArgumentParser(description="Utility to read data from thermodynamic database and write to MOOSE thermodynamic database (in JSON format)")
    parser.add_argument('-i', '--input', type=str, help="The input database", required=True)
    parser.add_argument('--format', type=str, help="The database format", choices=['gwb', 'eq36'], required=True)
    parser.add_argument('-o', '--output', type=str, default='moose_geochemdb.json', help="The output filename. Default: %(default)s")
    return parser.parse_args()


def main():
    """
    Parse the thermodynamic database and convert the data to
    the MOOSE JSON format

    Usage: database_converter.py -i filename --format format -o output_name

    For help text: database_converter.py --help
    """
    # Command-line options
    opt = command_line_options()

    # Read the entire database file into a list
    with open(opt.input, 'r') as dbfile:
        dblist = dbfile.readlines()

    # Parse data using appropriate reader
    if opt.format == 'gwb':
        db = gwb_reader.readDatabase(dblist)
    elif opt.format == 'eq36':
        db = eq36_reader.readDatabase(dblist)

    # Combine all of the database data into a single dictionary
    database = {}
    # Header information
    database['Header'] = {}
    database['Header']['title'] = 'MOOSE thermodynamic database'
    database['Header']['original'] = 'moose' + os.path.abspath(opt.input).split('moose')[1]
    database['Header']['date'] = datetime.datetime.now().strftime("%H:%M %d-%m-%Y")
    database['Header']['original format'] = db.format
    if db.header:
        database['Header']['original header'] = db.header
    if db.activity_model:
        database['Header']['activity model'] = db.activity_model
    if db.fugacity_model:
        database['Header']['fugacity model'] = db.fugacity_model
    if db.logk_model:
        database['Header']['logk model'] = db.logk_model
    if db.logk_model_eqn:
        database['Header']['logk model (functional form)'] = db.logk_model_eqn
    database['Header']['temperatures'] = db.temperatures
    database['Header']['pressures'] = db.pressures

    if db.activity_model == 'debye-huckel':
        database['Header']['adh'] = db.adh
        database['Header']['bdh'] =db.bdh
        database['Header']['bdot'] =db.bdot

    if db.neutral_species:
        database['Header']['neutral species'] = db.neutral_species

    if db.elements:
        database['elements'] = db.elements
    if db.basis_species:
        database['basis species'] = db.basis_species
    if db.secondary_species:
        database['secondary species'] = db.secondary_species
    if db.free_electron:
        database['free electron'] = db.free_electron
    if db.mineral_species:
        database['mineral species'] = db.mineral_species
    if db.gas_species:
        database['gas species'] = db.gas_species
    if db.redox_couples:
        database['redox couples'] = db.redox_couples
    if db.oxides:
        database['oxides'] = db.oxides
    if db.sorbing_minerals:
        database['sorbing minerals'] = db.sorbing_minerals
    if db.surface_species:
        database['surface species'] = db.surface_species

    # Write out the database to JSON format
    with open(opt.output, 'w') as output:
        json.dump(database, output, indent=2)

    sys.stdout.write("Finished parsing " + opt.input + ".  Output written to " + opt.output + "\n")
    sys.exit(0)

if __name__ == '__main__':
    main()
