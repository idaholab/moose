#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

"""
With only a filename given, this script runs each unittest in a file in a separate process.
If the test name is given as well, it will only run that test.
"""
import sys, os
import unittest
from importlib import import_module
import argparse
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument("--file", "-f", dest="pyfile", help="File to load tests from. Should be in the current working directory.", required=True)
parser.add_argument("--buffer", "-b", dest="buffer", action="store_true", help="Buffer stdout")
parser.add_argument("--verbosity", "-v", dest="verbosity", type=int, default=2, help="Set the verbosity level")
parser.add_argument("--test", "-t", dest="test", help="Run an individual test")

parsed = parser.parse_args(sys.argv[1:])
module_name = os.path.splitext(parsed.pyfile)[0]
# make sure we can load the file
sys.path.insert(0, os.getcwd())
mod = import_module(module_name)
loader = unittest.TestLoader()
tests = loader.loadTestsFromModule(mod)

if tests.countTestCases() == 0:
    print("%s had no unit tests" % parsed.pyfile)
    sys.exit(1)

all_tests = {}
for test in tests:
    for t in test:
        all_tests[str(t)] = t

if parsed.test:
    # A test name was passed in, so just try to run it.
    if parsed.test in all_tests:
        test_to_run = all_tests[parsed.test]
        # Apparently the setupClass/tearDownClass aren't called when running an individual test
        test_to_run.setUpClass()
        runner = unittest.TextTestRunner(verbosity=parsed.verbosity, buffer=parsed.buffer)
        results = runner.run(test_to_run)
        test_to_run.tearDownClass()
        sys.exit(int(not results.wasSuccessful()))
    else:
        print("%s not found in %s" % (parsed.test, parsed.pyfile))
        sys.exit(1)
else:
    # No test name passed in. Break out each test and run them in their own process
    final_code = 0
    for name in all_tests.keys():
        cmd = [__file__, "-f", parsed.pyfile, "-v", str(parsed.verbosity), "-t", name]
        if parsed.buffer:
            cmd.append("-b")
        ret = subprocess.call(cmd)
        if ret != 0:
            print("%s exited %s" % (name, ret))
            final_code = 1
    sys.exit(final_code)
