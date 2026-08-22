# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details

import signal
import subprocess
import sys
import textwrap
import unittest
from tempfile import SpooledTemporaryFile

from TestHarness.runners.SubprocessRunner import SubprocessRunner


class TestSubprocessRunner(unittest.TestCase):
    @unittest.skipUnless(hasattr(signal, "SIGUSR1"), "SIGUSR1 is unavailable")
    def testSendSignalWaitsForReadyOutput(self):
        script = textwrap.dedent("""
            import signal
            import sys
            import time

            ready = False

            def signal_handler(_signal, _frame):
                sys.exit(0 if ready else 2)

            signal.signal(signal.SIGUSR1, signal_handler)
            print("startup output", flush=True)
            # Keep the process in its unready state long enough for an
            # incorrectly gated signal to arrive reliably.
            time.sleep(0.25)
            ready = True
            print("ready for signal", flush=True)
            # Bound the test if the parent fails to send the signal.
            time.sleep(10)
            sys.exit(3)
            """)

        runner = object.__new__(SubprocessRunner)
        runner.outfile = SpooledTemporaryFile(max_size=1000000)
        runner.process = subprocess.Popen(
            [sys.executable, "-c", script],
            stdout=runner.outfile,
            stderr=subprocess.DEVNULL,
        )
        try:
            runner.sendSignal(signal.SIGUSR1, "ready for signal")
            self.assertEqual(runner.process.wait(timeout=5), 0)
        finally:
            if runner.process.poll() is None:
                runner.process.kill()
                runner.process.wait()
            runner.outfile.close()


if __name__ == "__main__":
    unittest.main()
