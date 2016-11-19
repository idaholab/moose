import MooseDocs
import logging
log = logging.getLogger(__name__)


def check_options(parser, subparser):
  """
  Command-line options for check command.
  """

  check_parser = subparser.add_parser('check', help="Check that the documentation exists and is complete for your application.")
  check_parser.add_argument('--locations', nargs='+', help="List of locations to consider, names should match the keys listed in the configuration file.")
  return check_parser


def check(**kwargs):
  """
  Performs checks for missing documentation.
  """

  MooseDocs.commands.generate(generate=False, **kwargs)
