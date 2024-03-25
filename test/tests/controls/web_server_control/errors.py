#!/usr/bin/env python3
from base_controller import *
import csv
import math

if __name__ == '__main__':
    if len(sys.argv) < 2:
        raise Exception('Missing test command line argument')
    test = sys.argv[1]

    def run_control(control):
        control.wait('INITIAL')

        if test == 'set_controllable_no_exist':
            control.setControllableReal('no_exist', 0)
        elif test == 'postprocessor_no_exist':
            control.getPostprocessor('no_exist')
        elif test == 'set_controllable_unregistered_type':
            control._setControllable('unused', 'BadType', 'unused')

        control.setContinue()

        if test == 'continue_not_waiting':
            control.setContinue()
        elif test == 'get_postprocessor_not_waiting':
            control.getPostprocessor('unused')
        elif test == 'set_controllable_not_waiting':
            control.setControllableReal('unused', 0)

    base_controller('web_server', run_control)
