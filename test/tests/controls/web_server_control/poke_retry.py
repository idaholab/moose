#!/usr/bin/env python3
# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test that a transient poke failure is retried instead of killing the run."""

from time import sleep
from unittest.mock import patch

from moosecontrol.runners.utils import Poker
from requests import ConnectionError
from testmoosecontrol import TestMooseControl

# Number of poke attempts to fail (with a transient error) before
# letting them succeed again
NUM_FAILURES = 3

_real_poke = Poker.poke
_failures_remaining = [NUM_FAILURES]


def _flaky_poke(self):
    """Fail the first few pokes to simulate a transient webserver hiccup."""
    if _failures_remaining[0] > 0:
        _failures_remaining[0] -= 1
        raise ConnectionError("Simulated bad status line")
    return _real_poke(self)


# This should be called by the test harness with the wait_initial.i input
# file. It makes the poke thread fail a few times in a row and then checks
# that the run isn't killed by the client_timeout below; prior to retrying
# failed pokes, a single failed poke would stop the poke thread for good,
# eventually causing MOOSE to fatally error out with a client_timeout.
if __name__ == "__main__":
    with (
        patch.object(Poker, "poke", _flaky_poke),
        TestMooseControl(
            "web_server", runner_kwargs={"poke_poll_time": 0.02}
        ) as control,
    ):
        control.wait("INITIAL")

        # Sit idle for longer than the client_timeout set below (twice
        # over); if the poke thread had stopped instead of retrying,
        # MOOSE would kill the simulation well before this returns
        sleep(1)

        control.set_continue()
