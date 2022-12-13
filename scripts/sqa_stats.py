#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

"""
Report statistics regarding the SQA documentation in MOOSE.
"""
import mooseutils

if __name__ == '__main__':
    data = mooseutils.SQAStats('Total')
    data += mooseutils.compute_requirement_stats('test/tests', list_missing=True)
    data += mooseutils.compute_requirement_stats('stork')
    data += mooseutils.compute_requirement_stats('tutorials')
    data += mooseutils.compute_requirement_stats('examples')
    data += mooseutils.compute_requirement_stats('scripts')
    data += mooseutils.compute_requirement_stats('python')
    data += mooseutils.compute_requirement_stats('modules/chemical_reactions')
    data += mooseutils.compute_requirement_stats('modules/combined')
    data += mooseutils.compute_requirement_stats('modules/contact')
    data += mooseutils.compute_requirement_stats('modules/external_petsc_solver')
    data += mooseutils.compute_requirement_stats('modules/fluid_properties')
    data += mooseutils.compute_requirement_stats('modules/fsi')
    data += mooseutils.compute_requirement_stats('modules/functional_expansion_tools')
    data += mooseutils.compute_requirement_stats('modules/geochemistry')
    data += mooseutils.compute_requirement_stats('modules/heat_conduction')
    data += mooseutils.compute_requirement_stats('modules/level_set')
    data += mooseutils.compute_requirement_stats('modules/misc')
    data += mooseutils.compute_requirement_stats('modules/navier_stokes')
    data += mooseutils.compute_requirement_stats('modules/optimization')
    data += mooseutils.compute_requirement_stats('modules/phase_field')
    data += mooseutils.compute_requirement_stats('modules/porous_flow')
    data += mooseutils.compute_requirement_stats('modules/rdg')
    data += mooseutils.compute_requirement_stats('modules/reactor')
    data += mooseutils.compute_requirement_stats('modules/richards')
    data += mooseutils.compute_requirement_stats('modules/scalar_transport')
    data += mooseutils.compute_requirement_stats('modules/solid_mechanics')
    data += mooseutils.compute_requirement_stats('modules/solid_properties')
    data += mooseutils.compute_requirement_stats('modules/stochastic_tools')
    data += mooseutils.compute_requirement_stats('modules/tensor_mechanics')
    data += mooseutils.compute_requirement_stats('modules/xfem')
    print data

    tdata = mooseutils.compute_requirement_stats('', show=False)
    if tdata.files != data.files:
        msg = 'ERROR: The total number of test files for the supplied directories does not match\n' \
              '       the  number of test files in the repository.'
        print mooseutils.colorText(msg, 'RED')
