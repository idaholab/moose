import os
import unittest
import MooseDocs

def test_options(parser, subparser):
  """
  Command-line options for test command.
  """
  test_parser = subparser.add_parser('test', help='Performs unitesting of MooseDocs module')
  test_parser.add_argument('--pattern', default='test*.py', help="The test name pattern to search.")
  test_parser.add_argument('--start-dir', default='tests', help="The location of the tests directory.")
  return test_parser


def test(pattern=None, start_dir=None, **kwargs):
  """
  Runs MooseDocs unittests.
  """
  os.chdir(start_dir)
  loader = unittest.TestLoader()
  suite = loader.discover(os.getcwd(), pattern)
  runner = unittest.TextTestRunner(verbosity=2)
  runner.run(suite)
