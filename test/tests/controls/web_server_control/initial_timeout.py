#!/usr/bin/env python3
# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test the WebServerControl timing out on initial."""

from sys import exit

from moosecontrol.runners.interfaces import SubprocessRunnerInterface
from testmoosecontrol import TestMooseControl

if __name__ == "__main__":
    runner = TestMooseControl("web_server").control.runner

    # Just start the process, nothing else because
    # the BaseRunner will poke once it is listening
    SubprocessRunnerInterface.initialize(runner)
    SubprocessRunnerInterface.finalize(runner)

    exit(runner.get_return_code())
