import os
import unittest
import MooseDocs

def test_options(parser, subparser):
    """
    Command-line options for test command.
    """
    test_parser = subparser.add_parser('test', help='Performs unitesting of MooseDocs module')
    test_parser.add_argument('--pattern', default='test*.py', help="The test name pattern to search.")
    return test_parser


def test(pattern=None, **kwargs):
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
