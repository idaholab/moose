#pylint: disable=missing-docstring, no-member
import os
import logging
import time
import cProfile as profile
import pstats
import StringIO

import anytree

from MooseDocs import common
from MooseDocs.tree import page

def command_line_options(subparser, parent):
    """Define the 'profile' command."""

    parser = subparser.add_parser('profile',
                                  parents=[parent],
                                  help='Tool for profiling MooseDocs performance.')
    parser.add_argument('--config', default='config.yml', help="The configuration file.")
    parser.add_argument('--destination',
                        default=os.path.join(os.getenv('HOME'), '.local', 'share', 'moose', 'site'),
                        help="Destination for writing build content.")

    parser.add_argument('-t', '--tokenize', action='store_true',
                        help="Enable profiling of tokenization.")
    parser.add_argument('-r', '--render', action='store_true',
                        help="Enable profiling of rendering.")

def main(options):
    """./moosedocs.py profile"""

    log = logging.getLogger(__name__)
    translator, _ = common.load_config(options.config)
    translator.init(options.destination)

    if options.tokenize:
        log.info('Profiling tokenization, this may take several minutes.')
        _run_profile(translator, _tokenize)

    if options.render:
        log.info('Performing tokenization...')
        _tokenize(translator)

        log.info('Profiling rendering, this may take several minutes.')
        _run_profile(translator, _render)

def _tokenize(translator):
    func = lambda n: isinstance(n, page.MarkdownNode)
    for node in anytree.PreOrderIter(translator.root, filter_=func):
        translator.current = node
        translator.reinit()
        node.tokenize()
        translator.current = None

def _render(translator):
    func = lambda n: isinstance(n, page.MarkdownNode)
    for node in anytree.PreOrderIter(translator.root, filter_=func):
        translator.current = node
        node.render(node.ast)
        translator.current = None

def _run_profile(translator, function):
    pr = profile.Profile()
    start = time.time()
    pr.runcall(function, translator)
    print 'Total Time:', time.time() - start
    s = StringIO.StringIO()
    ps = pstats.Stats(pr, stream=s).sort_stats('tottime')
    ps.print_stats()
    print s.getvalue()
