#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import csv

from testmoosecontrol import TestMooseControl, expect_close

# This should be called by the test harness with the get_postprocessor.i
# input file to obtain changing postprocessor values from the web server
if __name__ == '__main__':
    # Open the gold file associated with this test, which contains the
    # results of the postprocessor that we're trying to read from
    with open('gold/get_postprocessor_out.csv', 'r', encoding='utf-8') as f:
        gold = []
        for row in list(csv.reader(f))[1:]:
            gold.append(float(row[1]))

    with TestMooseControl('web_server') as control:
        control.wait('INITIAL')

        # Get the initial value of the postprocessor and compare to the gold
        value = control.get_postprocessor('t')
        expect_close(value, gold[0])

        # Tell MOOSE to continue with the solve
        control.set_continue()

        # Control through the timesteps (determined by length of the gold file)
        for t in range(len(gold) - 1):
            # Wait, where we shold be at TIMESTEP_BEGIN
            control.wait('TIMESTEP_BEGIN')

            # Get the current value of the postprocessor and compare to the gold
            value = control.get_postprocessor('t')
            expect_close(value, gold[t])

            # Tell MOOSE to continue with the solve
            control.set_continue()
