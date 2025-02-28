from printind.printind_decorators import printi_all_method_calls as printidc
from printind.printind_function import printi, printiv
from tensorforce.environments import Environment
import tensorforce
from tqdm import tqdm
import numpy as np
import matplotlib.pyplot as plt

# a bit hacky, but meeehh... FIXME!!
import sys
import os
cwd = os.getcwd()
sys.path.append(cwd + "/../Simulation/")

from dolfin import Expression, File, plot
from probes import PenetratedDragProbeANN, PenetratedLiftProbeANN, PressureProbeANN, VelocityProbeANN, RecirculationAreaProbe
from generate_msh import generate_mesh
from flow_solver import FlowSolver
from msh_convert import convert
from dolfin import *

import numpy as np
import os
import random as random

import pickle

import time
import math
import csv

import shutil

# @printidc()
class MyEnv(Environment):
    """Environment for 2D flow simulation around a cylinder."""

    def __init__(self, output_params,
                 optimization_params, inspection_params,
                 size_history=2000,
                 size_time_state=50,
                 number_steps_execution=1):

        printi("--- call init ---")

        self.output_params = output_params
        self.optimization_params = optimization_params
        self.inspection_params = inspection_params
        self.size_history = size_history
        self.size_time_state = size_time_state
        self.number_steps_execution = number_steps_execution

        self.last_episode_number = 0
        self.episode_number = 0

        self.episode_measurement = np.array([])

        self.start_class(complete_reset=True)

        printi("--- done init ---")

    def start_class(self, complete_reset=True):

		self.solver_step = 0
		self.accumulated_measurement = 0

		self.history_parameters = {}

		self.history_parameters["action"] = RingBuffer(self.size_history)
        self.history_parameters["observation"] = RingBuffer(self.size_history)
		self.history_parameters["measurement"] = RingBuffer(self.size_history)

		# ------------------------------------------------------------------------
		# create the flow simulation object
		self.flow = MyFunction

		# ------------------------------------------------------------------------
		# Setup probes
	    self.obs_probes = PressureProbeANN(self.flow, self.output_params['locations'])
		self.meas_probe = PenetratedDragProbeANN(self.flow)

		# ------------------------------------------------------------------------
		# No flux from jets for starting
		self.action = 0.0

		self.ready_to_use = True

    def write_history_parameters(self):
        self.history_parameters["action"].extend(self.action)
        self.history_parameters["observation"].extend(self.obs_probes)
        self.history_parameters["measurement"].extend(self.meas_probes)

    def __str__(self):
        printi("Env2DCylinder ---")

    def close(self):
        """
        Close environment. No other method calls possible afterwards.
        """

        self.ready_to_use = False

    def seed(self, seed):
        """
        Sets the random seed of the environment to the given value (current time, if seed=None).
        Naturally deterministic Environments don't have to implement this method.

        Args:
            seed (int): The seed to use for initializing the pseudo-random number generator (default=epoch time in sec).
        Returns: The actual seed (int) used OR None if Environment did not override this method (no seeding supported).
        """

        # we have a deterministic environment: no need to implement

        return None

    def reset(self):
        """
        Reset environment and setup for new episode.

        Returns:
            initial state of reset environment.
        """

        if self.solver_step > 0:
            mean_accumulated_drag = self.accumulated_drag / self.solver_step
            mean_accumulated_lift = self.accumulated_lift / self.solver_step

            if self.verbose > -1:
                printi("mean accumulated drag on the whole episode: {}".format(mean_accumulated_drag))

        if self.inspection_params["show_all_at_reset"]:
            self.show_drag()
            self.show_control()

        self.start_class(complete_reset=True)

        next_state = np.transpose(np.array(self.probes_values))

        return(next_state)

    def execute(self, actions=None):

        printi("--- call execute ---")

        if actions:
            actions = 0.0

        printiv(actions)

        # to execute several numerical integration steps
        for _ in range(self.number_steps_execution):

            self.Qs = np.transpose(np.array(actions))

            # evolve one numerical timestep forward
            self.u_, self.p_ = self.flow.evolve(self.Qs)

            # we have done one solver step
            self.solver_step += 1

            # sample probes and drag
            self.observations = self.ann_probes.sample(self.u_, self.p_).flatten()
            self.measurements = self.drag_probe.sample(self.u_, self.p_)

            # write to the history buffers
            self.write_history_parameters()

            self.accumulated_measurement += self.measurements

        next_state = np.transpose(np.array(self.probes_values))

        if self.verbose > 2:
            printiv(next_state)

        terminal = False

        reward = self.compute_reward()

        if self.verbose > 1:
            printi("--- done execute ---")

        return(next_state, terminal, reward)

    def compute_reward(self):

        avg_length = min(500, self.number_steps_execution)
        avg_drag = np.mean(self.history_parameters["drag"].get()[-avg_length:])
        avg_lift = np.mean(self.history_parameters["lift"].get()[-avg_length:])
        return avg_drag + 0.159 - 0.2 * abs(avg_lift)

    @property
    def states(self):

        return dict(type='float',
                    shape=(1, )
                    )

    @property
    def actions(self):

        return dict(type='float',
                    shape=(1, ),
                    min_value=[-0.2],
                    max_value=[0.2])
