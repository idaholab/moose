#!/usr/bin/env python3
from base_controller import *

if __name__ == '__main__':
    if len(sys.argv) < 2:
        raise Exception('Missing test command line argument')
    test = sys.argv[1]

    def run_control(control):
        control.wait()

        if test == 'set_controllable_no_exist':
            control.setControllableReal('no_exist', 0)
        elif test == 'postprocessor_no_exist':
            control.getPostprocessor('no_exist')
        elif test == 'set_controllable_unregistered_type':
            control._setControllable('unused', 'BadType', 'unused')
        elif test == 'set_controllable_bad_convert':
            control._setControllable('Outputs/json/enable', 'bool', 'foo')
        elif test == 'set_controllable_vector_non_array':
            control._setControllable('Reporters/test/vec_real_value', 'std::vector<Real>', 1234)

    base_controller('web_server', run_control)
