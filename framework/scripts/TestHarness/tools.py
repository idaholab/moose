import sys, os

plugin_dir = '/scripts/TestHarness/testers'

module_path = os.path.dirname(__file__)
if os.environ.has_key("MOOSE_DIR"):
  MOOSE_DIR = os.environ['MOOSE_DIR']
else:
  MOOSE_DIR = os.path.abspath(module_path) + '/..'
sys.path.append(MOOSE_DIR + '/scripts/common')
sys.path.append(MOOSE_DIR + plugin_dir)

# Import the Two Harness classes
from TestTimer import TestTimer
from TestHarness import TestHarness

# Tester Base Class
from Tester import Tester
# New Testers can be created and automatically registered with the
# TestHarness if you follow these two steps:
# 1. Create a class inheriting from "Tester" or any of its descendents
# 2. Place your new class in the "plugin" directory under your application

def runTests(argv, app_name, moose_dir):
  if '--store-timing' in argv:
    # Pass control to TestTimer class for Test Timing
    harness = TestTimer(argv, app_name, moose_dir)
  else:
    harness = TestHarness(argv, app_name, moose_dir)

  # Get a reference to the factory out of the TestHarness
  factory = harness.getFactory()

  # TODO: We need to cascade the testers so that each app can use any
  # tester available in each parent application
  dirs = [os.path.abspath(os.path.dirname(sys.argv[0])), MOOSE_DIR]

  # Load the tester plugins into the factory reference
  factory.loadPlugins(dirs, plugin_dir, Tester, factory)

  # Finally find and run the tests
  harness.findAndRunTests()

