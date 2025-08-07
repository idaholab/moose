#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from TestHarnessTestCase import TestHarnessTestCase

from typing import Optional
import unittest
from dataclasses import dataclass

class TestAugmentedCapabilities(TestHarnessTestCase):
    def test(self):
        @dataclass
        class TestCase:
            capabilities: str
            skip: bool
            params: Optional[dict] = None

        def run(*cases: tuple, cli_args: Optional[list[str]] = []) -> dict:
            test_spec = {}
            for i, options in enumerate(cases):
                case = TestCase(*options)
                test_name = f'test{i}'
                test_spec[test_name] = {'type': 'RunApp',
                                        'input': 'unused',
                                        'should_execute': False,
                                        'capabilities': f'"moosetestapp & {case.capabilities}"'}
                if case.params:
                    test_spec[test_name].update(case.params)

            result = self.runTests(*cli_args, tests=test_spec, no_capabilities=False)
            harness = result.harness

            for i, options in enumerate(cases):
                case = TestCase(*options)
                test_name = f'test{i}'
                job = [j for j in harness.finished_jobs if j.getTestNameShort() == test_name][0]
                self.assertEqual(job.getStatus(), job.skip if case.skip else job.finished)

        # Basic choices that match a command line option
        for option in ['valgrind', 'recover', 'heavy']:
            # Option isn't set: capability '!option' will be ran and capability 'option' won't
            run((f'!{option}', False), (option, True))
            # Option is set: capability 'option' will be ran and capability '!option' won't
            params = {'heavy': True} if option == 'heavy' else {}
            run((f'{option}', False, params), (f'!{option}', True, params), cli_args=[f'--{option}'])

        # MPI procs
        run(('mpi_procs>1', True), ('mpi_procs=1', False))
        run(('mpi_procs>1', False), ('mpi_procs=1', True), cli_args=['-p', '2'])

        # Num threads
        run(('num_threads>1', True), ('num_threads=1', False))
        run(('num_threads>1', False), ('num_threads=1', True), cli_args=['--n-threads', '2'])

        # Device
        run(('compute_device=cpu', False), ('compute_device=foo', True))

if __name__ == '__main__':
    unittest.main()
