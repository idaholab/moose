#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from subprocess import Popen, PIPE
from unittest import main

from common import CaptureLogTestCase, setup_moose_python_path
setup_moose_python_path()

from moosecontrol.runners.utils.subprocessreader import SubprocessReader

class TestSubprocessReader(CaptureLogTestCase):
    """
    Tests for moosecontrol.utils.subprocessreader.SubprocessReader.
    """
    def test(self):
        """
        Tests running the SubprocessReader.
        """
        output = ['foo', 'bar']
        command = f'sleep 0.01 && echo "{"\n".join(output)}"'
        process = Popen(
            command,
            shell=True,
            stdout=PIPE,
            text=True,
            universal_newlines=True,
            bufsize=1
        )

        reader = SubprocessReader(process)
        reader.start()
        reader.join()
        process.wait()

        log_size = 2 + len(output)
        self.assertLogSize(log_size)
        self.assertLogMessage(0, 'Subprocess reader started')
        for i, line in enumerate(output):
            self.assertLogMessage(i + 1, SubprocessReader.OUTPUT_PREFIX + output[i])
        self.assertLogMessage(log_size - 1, 'Subprocess reader ending')

if __name__ == '__main__':
    main()
