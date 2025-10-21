#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# pylint: skip-file
# type: ignore

from subprocess import Popen, PIPE

from common import MooseControlTestCase, setup_moose_python_path
setup_moose_python_path()

from moosecontrol.runners.utils import SubprocessReader

class TestSubprocessReader(MooseControlTestCase):
    """
    Tests moosecontrol.runners.utils.subprocessreader.SubprocessReader.
    """
    def test(self):
        """
        Tests running the SubprocessReader.
        """
        output = ['foo', 'bar']
        joined_output = '\n'.join(output)
        command = f'sleep 0.01 && echo "{joined_output}"'
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
        self.assert_log_size(log_size)
        self.assert_log_message(0, 'Subprocess reader started')
        for i, line in enumerate(output):
            self.assert_log_message(i + 1, SubprocessReader.OUTPUT_PREFIX + output[i])
        self.assert_log_message(log_size - 1, 'Subprocess reader ending')
