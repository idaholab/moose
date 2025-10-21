#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# pylint: disable=invalid-name
import sys
import numpy as np

from test_moose_control import TestMooseControl

# This should be called by the test harness with the control_reporter.i
# input file to test setting the controllable values for many times. It
# takes a single command line argument which is the name of the test to run
if __name__ == '__main__':
    # Get the command line argument that is the test to run
    if len(sys.argv) < 2:
        raise ValueError('Missing value command line argument')
    controllable_name = sys.argv[1]

    # Path to the controllable value
    controllable_path = f'Reporters/test/{controllable_name}'

    # Determine what to set the values to and what function to call
    # on MooseControl to change the value
    match controllable_name:
        case 'bool_value':
            test_values = [False, True]
            method_name = 'set_controllable_bool'
        case 'real_value':
            test_values = [1.5, 3.2]
            method_name = 'set_controllable_real'
        case 'int_value':
            test_values = [-5000, 1]
            method_name = 'set_controllable_int'
        case 'vec_real_value':
            test_values = [[1, 2, 3, 4, 5], [6, 100]]
            method_name = 'set_controllable_vector_real'
        case 'vec_int_value':
            test_values = [[-1, -2, -3, 4, 5], [888, -1000]]
            method_name = 'set_controllable_vector_int'
        case 'string_value':
            test_values = ['foo', 'bar']
            method_name = 'set_controllable_string'
        case 'vec_string_value':
            test_values = [['what', 'magic'], ['dont', '   you', ' say!']]
            method_name = 'set_controllable_vector_string'
        case 'matrix_value':
            test_values = [np.arange((i + 1) * (i + 2), dtype=np.float64)
                           .reshape((i + 1, i + 2)) * 10**i for i in range(1, 3)]
            method_name = 'set_controllable_matrix'
        case _:
            raise ValueError(f'Unknown controllable name {controllable_name}')

    with TestMooseControl('web_server') as control:
        # Work through as many timesteps as we have values
        for test_value in test_values:
            # Hold until waiting, where we should be at TIMESTEP_END
            control.wait('TIMESTEP_END')

            # Get the function on the MooseControl that we should call
            method = getattr(control, method_name)
            # Call the function, which sets the controllable value
            method(controllable_path, test_value)

            # Tell MOOSE to continue with the solve
            control.set_continue()
