import os
import unittest
import MooseDocs
import logging
log = logging.getLogger(__name__)

def test_options(parser):
    """
    Command-line options for test command.
    """
    parser.add_argument('--pattern', default='test*.py', help="The test name pattern to search.")

def test(pattern=None):
    """
    Runs MooseDocs unittests.
    """
    if not os.path.exists('tests'):
        print 'No {} directory located.'.format('tests')
    else:
        os.chdir('tests')
        loader = unittest.TestLoader()
        suite = loader.discover(os.getcwd(), pattern)
        runner = unittest.TextTestRunner(verbosity=2)
        result = runner.run(suite)
        if not result.wasSuccessful():
            return 1
    return 0
