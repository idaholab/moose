#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from testmoosecontrol import TestMooseControl

# This should be called by the test harness with the errors.i
# input file (which just waits on INITIAL) to test connecting
# to the web server on a port
if __name__ == '__main__':
    with TestMooseControl('web_server', use_port=True) as control:
        control.wait('INITIAL')
        control.set_continue()
