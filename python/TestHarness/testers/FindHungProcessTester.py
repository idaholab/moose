from RunApp import RunApp
import os, sys, subprocess

class FindHungProcessTester(RunApp):

    @staticmethod
    def validParams():#this is a constructor
        params = RunApp.validParams()
        return params

    def getCommand(self, options):
        # Create the command line string to run
        return "echo this is working right now; mpiexec -n 4 ./bad_mpi -s 2; python ../../../framework/scripts/find_hung_process.py -v test bad_mpi --test-local -qp"
        #return "echo is this; sleep 1; echo working?"

    def __init__(self, name, params):
        RunApp.__init__(self, name, params)


