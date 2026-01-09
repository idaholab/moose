#!/Users/rietaa/miniconda3/envs/moose/bin/python3

import numpy as np
import sys
import os
import unittest
from unittest import mock


if importlib.util.find_spec('moose_stochastic_tools') is None:
    _stm_python_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', 'python'))
    sys.path.append(_stm_python_path)

from moose_stochastic_tools import StochasticControl, StochasticRunOptions



class TestStochasticVectors(unittest.TestCase):
    input_file_name = 'main_runner.i'
    def __init__(self):
        self.opts = StochasticControl.StochasticRunOptions(
            num_procs=10,
            mpi_command="mpiexec",
            input_name='stochastic_run.i',
            multiapp_mode=StochasticControl.StochasticRunOptions.MultiAppMode.BATCH_RESET,
            ignore_solve_not_converge=False
        )
        self.runner = StochasticControl.StochasticControl(os.path.join(os.environ['MOOSE_DIR'],'modules','stochastic_tools','stochastic_tools-opt'),
                                         physics_input='main_runner.i',options=opts,
                                         parameters=['capsule1:Postprocessors/frequency_factor/value', 
                                            'capsule1:Postprocessors/activation_energy/value',
                                            'capsule1:BCs/heat_DRV_outer/value',
                                            'capsule1:mesh_specified'],
                        quantities_of_interest=['capsule_01/x',
                                                'capsule_01/temp',
                                                ])
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


