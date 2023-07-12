from RunApp import RunApp
import os, sys, subprocess

class FindHungProcessTester(RunApp):

    @staticmethod
    def validParams():#this is a constructor
        params = RunApp.validParams()
        return params

    def __init__(self, name, params):
        RunApp.__init__(self, name, params)


    def getCommand(self, options):
        # Create the command line string to run
        cmd_str = ""
        #num_cpus = RunApp.getProcs()
        num_cpus = '6'
        #num_stacks = RunApp.getThreads()
        num_stacks = '3'

        binary_name = 'bad_mpi'
        script_name = 'find_hung_process.py'
        script_path = '../../../framework/scripts/'#this only works if in my temp "testers" directory

        if ('mpiexec' not in sys.argv):
            cmd_str = cmd_str + 'mpiexec'+ ' '

        if ('mpiexec' in cmd_str) and ('-n' not in sys.argv):
            cmd_str = cmd_str + (f'-n {num_cpus}'+ ' ')

        if (binary_name not in sys.argv):
            cmd_str = cmd_str + (f'./{binary_name}'+ ' ')

        if (binary_name in cmd_str) and ('-s' not in sys.argv):
            cmd_str = cmd_str + (f'-s {num_stacks}' + ' ')

        cmd_str = cmd_str + ("&" + ' ')# end of first command, want to run bad_mpi in background

        if (f'./{binary_name}' in cmd_str):
            cmd_str = cmd_str + ('sleep 5 ;' + ' ')

        #now running find_hung_process.py script
        cmd_str = cmd_str + ('python' + ' ')

        if (script_name not in sys.argv):
            cmd_str = cmd_str + (f'{script_path}{script_name}' + ' ')

        if (script_name in cmd_str) and ('-v' not in cmd_str):
            cmd_str = cmd_str + ('-v' + ' ')

        if (script_name in cmd_str) and ('test' not in sys.argv):
            cmd_str = cmd_str + ('test' + ' ')#want to test script in "local test" mode without job queuer

        if ('test' in cmd_str):
            cmd_str = cmd_str + (f'{binary_name}' + ' ')

        if ('test' in cmd_str):
            cmd_str = cmd_str + ('--test-local' + ' ')

        if ('test' in cmd_str):
            cmd_str = cmd_str + ('-qp' + ' ')

        cmd_str = cmd_str + ('&&' + ' ')#condition where bad_mpi program will only be killed after the script runs.

        if (script_name in cmd_str) and ('pkill' not in sys.argv):
            cmd_str = cmd_str + (f'pkill -9 {binary_name}')

        return cmd_str
        #return f"mpiexec -n {num_cpus} ./bad_mpi -s {num_stacks} & sleep 5; python ../../../framework/scripts/find_hung_process.py -v test bad_mpi --test-local -qp && pkill -9 bad_mpi"

