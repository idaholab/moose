#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import sys
from testmoosecontrol import TestMooseControl

if __name__ == '__main__':
    if len(sys.argv) < 2:
        raise ValueError('Missing test command line argument')
    test = sys.argv[1]

    with TestMooseControl('web_server') as control:
        match test:
            case 'set_controllable_no_exist':
                control.set_real('no_exist', 0.0)
            case 'postprocessor_no_exist':
                control.get_postprocessor('no_exist')
            case 'set_controllable_unregistered_type':
                control.set_controllable('unused', 'BadType', (str,), 'unused')
            case 'set_controllable_bad_convert':
                control.set_controllable('Outputs/json/enable', 'bool', (str,), 'foo')
            case 'set_controllable_vector_non_array':
                control.set_controllable(
                    'Reporters/test/vec_real_value',
                    'std::vector<Real>',
                    (int,),
                    1234
                )
            case 'set_realeigenmatrix_bad_convert1':
                control.set_controllable(
                    'Reporters/test/matrix_value',
                    'RealEigenMatrix',
                    (int,),
                    1234
                )
            case 'set_realeigenmatrix_bad_convert2':
                control.set_controllable(
                    'Reporters/test/matrix_value',
                    'RealEigenMatrix',
                    (int,),
                    [1234]
                )
            case 'set_realeigenmatrix_jagged':
                control.set_controllable(
                    'Reporters/test/matrix_value',
                    'RealEigenMatrix',
                    (list,),
                    [[11, 12], [21]]
                )
            case _:
                raise ValueError(f'Unknown test {test}')
