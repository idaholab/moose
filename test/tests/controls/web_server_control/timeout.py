#!/usr/bin/env python3
# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test the timeout within WebServerControl."""

from sys import exit
from time import sleep

from testmoosecontrol import TestMooseControl

if __name__ == "__main__":
    with TestMooseControl(
        "web_server", runner_kwargs={"poke_poll_time": None}
    ) as control:
        runner = control.runner
        while runner.is_process_running():
            sleep(0.01)
        exit(runner.get_return_code())
