from RunApp import RunApp
import os, sys, subprocess

class FindHungProcessTester(RunApp):

    print('Hello in FindHungProcessTester class')


    # def command(self):
    #     cmd = self.getCommand(self.cli_command_options).split(" ")
    #     return cmd

    @staticmethod
    def validParams():
        params = RunApp.validParams()

        return params

    def __init__(self, name, params):
        RunApp.__init__(self, name, params)


# def main():
#    test = FindHungProcessTester

#    cmd = test.getCommand("echo test")
#    print(cmd)


