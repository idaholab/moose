#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import multiprocessing

class Barrier(object):
    """
    Implementation of a barrier.
    https://github.com/ghcollin/multitables/blob/master/multitables.py

    NOTE: The multiprocessing package includes a Barrier object as of python 3.3.
    """

    def __init__(self, n_procs):
        """
        Create a barrier that waits for n_procs processes.
        :param n_procs: The number of processes to wait for.
        """
        self.__n_procs = n_procs
        self.__count = multiprocessing.Value('i', 0, lock=False)
        self.__cvar = multiprocessing.Condition()

    def wait(self):
        """Wait until all processes have reached the barrier."""
        with self.__cvar:
            self.__count.value += 1
            self.__cvar.notify_all()
            while self.__count.value < self.__n_procs:
                self.__cvar.wait()

    def reset(self):
        """Re-set the barrier so that it can be used again."""
        with self.__cvar:
            self.__count.value = 0
