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
        cmd_list = ""
        #num_cpus = RunApp.getProcs()
        num_cpus = '6'
        #num_stacks = RunApp.getThreads()
        num_stacks = '3'

        binary_name = 'bad_mpi'
        script_name = 'find_hung_process.py'
        script_path = '../../../framework/scripts/'#this only works if in my temp "testers" directory

        cmd_list = f'chmod +x {binary_name}'+ ' ;'+ ' '

        if ('mpiexec' not in cmd_list):
            cmd_list = cmd_list + 'mpiexec'+ ' '

        if ('mpiexec' in cmd_list):
            cmd_list = cmd_list + (f'-n {num_cpus}'+ ' ')

        if ('mpiexec' in cmd_list):
            cmd_list = cmd_list + (f'./{binary_name}'+ ' ')

        if (binary_name in cmd_list):
            cmd_list = cmd_list + (f'-s {num_stacks}' + ' ')

        cmd_list = cmd_list + ("&" + ' ')# end of first command, want to run bad_mpi in background

        if (f'./{binary_name}' in cmd_list):
            cmd_list = cmd_list + ('sleep 5 ;' + ' ')

        #now running find_hung_process.py script
        cmd_list = cmd_list + ('python' + ' ')

        if (script_name not in cmd_list):
            cmd_list = cmd_list + (f'{script_path}{script_name}' + ' ')

        if (script_name in cmd_list):
            cmd_list = cmd_list + ('-v' + ' ')

        if (script_name in cmd_list) and ('test' not in cmd_list):
            cmd_list = cmd_list + ('test' + ' ')#want to test script in "local test" mode without job queuer

        if ('test' in cmd_list):
            cmd_list = cmd_list + (f'{binary_name}' + ' ')

        if ('test' in cmd_list):
            cmd_list = cmd_list + ('--test-local' + ' ')

        if ('test' in cmd_list):
            cmd_list = cmd_list + ('-qp' + ' ')

        cmd_list = cmd_list + ('&&' + ' ')#condition where bad_mpi program will only be killed after the script runs.

        if (script_name in cmd_list) and ('pkill' not in cmd_list):
            cmd_list = cmd_list + (f'pkill -9 {binary_name}')

        return cmd_list
