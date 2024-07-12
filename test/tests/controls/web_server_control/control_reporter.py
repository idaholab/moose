#!/usr/bin/env python3
from base_controller import *
import sys

# This should be called by the test harness with the control_reporter.i
# input file to test setting the controllable values for many times. It
# takes a single command line argument which is the name of the test to run
if __name__ == '__main__':
    # Get the command line argument that is the test to run
    if len(sys.argv) < 2:
        raise Exception('Missing value command line argument')
    value = sys.argv[1]

    # Path to the controllable value
    path = f'Reporters/test/{value}'
    # Determine what to set the values to and what function to call
    # on MooseControl to change the value
    if value == 'bool_value':
      values = [False, True]
      set_name = 'setControllableBool'
    elif value == 'real_value':
      values = [1.5, 3.2]
      set_name = 'setControllableReal'
    elif value == 'int_value':
      values = [-5000, 1]
      set_name = 'setControllableInt'
    elif value == 'vec_real_value':
      values = [[1, 2, 3, 4, 5], [6, 100]]
      set_name = 'setControllableVectorReal'
    elif value == 'vec_int_value':
      values = [[-1, -2, -3, 4, 5], [888, -1000]]
      set_name = 'setControllableVectorInt'
    elif value == 'string_value':
      values = ['foo', 'bar']
      set_name = 'setControllableString'
    elif value == 'vec_string_value':
      values = [['what', 'magic'], ['dont', '   you', ' say!']]
      set_name = 'setControllableVectorString'
    else:
        raise Exception(f'Unknown value {value}')

    # Passed into the base controller to run the MooseControl
    def run_control(control):
        # Work through as many timesteps as we have values
        for value in values:
            # Hold until waiting, where we should be at TIMESTEP_END
            control.wait('TIMESTEP_END')

            # Get the function on the MooseControl that we should call
            function = getattr(control, set_name)
            # Call the function, which sets the controllable value
            function(path, value)

            # Tell MOOSE to continue with the solve
            control.setContinue()

    # Leverages base_controller.py to execute moose based on the
    # command from the test harness and instantiate the MooseControl
    base_controller('web_server', run_control)
