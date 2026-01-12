import numpy as np
import sys
import os
import unittest
import importlib
from unittest import mock

if importlib.util.find_spec('moose_stochastic_tools') is None:
    _stm_python_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', 'python'))
    sys.path.append(_stm_python_path)

from moose_stochastic_tools import StochasticControl, StochasticRunOptions
from moose_stochastic_tools.StochasticControl import StochasticRunner


class TestStochasticVectors(unittest.TestCase):
    input_file_name = 'main_runner.i'
    def __init__(self,*args,**kwargs):
        super().__init__(*args,**kwargs)
        self.opts = StochasticRunOptions(
            num_procs=10,
            mpi_command="mpiexec",
            input_name='stochastic_run.i',
            multiapp_mode=StochasticRunOptions.MultiAppMode.BATCH_RESET,
            ignore_solve_not_converge=False
        )
        self.control = StochasticControl(os.path.join(os.environ['MOOSE_DIR'],'modules','stochastic_tools','stochastic_tools-opt'),
                         physics_input='vpp_test_runner.i',options=self.opts,
                         parameters=['capsule1:Postprocessors/frequency_factor/value',
                            'capsule1:Postprocessors/activation_energy/value',
                            'capsule1:BCs/heat_DRV_outer/value',
                            'capsule1:mesh_specified'],
                         quantities_of_interest=['capsule_01/x',
                                                'capsule_01/temp',
                                        ])
    def setUp(self):
        self.runner = self.control.__enter__()

    def tearDown(self):
        self.control.__exit__()
        del self.runner

    def testVectorConfig(self):
        self.runner.configCache(tol=[1,1e-10,1e-14,1e-14])

    def objective(self,params):
        return self.runner(params)

    def testListFormat(self):
        self.objective([335478,27700,500,1])

    def testCachedList(self):
        self.objective([335478,27700,500,1])

    def testBatchList(self):
        self.objective([[385478,29700,385,1],
                     [335478,27700,500,1],
                     [485478,21700,385,1],
                     [365478,23700,385,1],
                     ]) # Vector of samples simultaneous ok with list output and caching?

    def testArrayFormat(self):
        self.objective([385478,27700,500,20]) # Test vector compatibility

    def testVectorArray(self):
        self.objective([[385478,27700,500,20],
               [315478,27700,500,20]]) # Test vector compatibility with array output


