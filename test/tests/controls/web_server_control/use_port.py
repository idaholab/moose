#!/usr/bin/env python3
from base_controller import *
import sys

# This should be called by the test harness with the errors.i
# input file (which just waits on INITIAL) to test connecting
# to the web server on a port
if __name__ == '__main__':
    # Passed into the base controller to run the MooseControl
    def run_control(control):
        # Just need to wait on initial
        control.wait('INITIAL')
        control.setContinue()

    # Leverages base_controller.py to execute moose based on the
    # command from the test harness and instantiate the MooseControl
    base_controller('web_server', run_control, use_port=True)
