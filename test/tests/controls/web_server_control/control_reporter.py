#!/usr/bin/env python3
from base_controller import *
import sys

if __name__ == '__main__':
    if len(sys.argv) < 2:
        raise Exception('Missing value command line argument')
    value = sys.argv[1]

    path = f'Reporters/test/{value}'
    if value == 'real_value':
      values = [1.5, 3.2]
      set_name = 'setControllableReal'
    elif value == 'vec_real_value':
      values = [[1, 2, 3, 4, 5], [6, 100]]
      set_name = 'setControllableVectorReal'
    elif value == 'string_value':
      values = ['foo', 'bar']
      set_name = 'setControllableString'
    elif value == 'vec_string_value':
      values = [['what', 'magic'], ['dont', '   you', ' say!']]
      set_name = 'setControllableVectorString'
    else:
        raise Exception(f'Unknown value {value}')

    def run_control(control):
        for value in values:
            control.wait('TIMESTEP_END')

            function = getattr(control, set_name)
            function(path, value)

            control.setContinue()

    base_controller('web_server', run_control)
