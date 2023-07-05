import RunApp
from RunApp import RunApp
import os, sys, subprocess

class FindHungProcessTester(RunApp):

    print('Hello in FindHungProcessTester class')

    def __init__(self):
        self.cli_command_options =""

    def command(self):
        cmd = self.getCommand(self.cli_command_options).split(" ")
        return cmd


def main():
   test = FindHungProcessTester

   cmd = test.getCommand("echo test")
   print(cmd)
