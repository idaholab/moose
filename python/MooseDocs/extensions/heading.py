#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import logging
import moosetree
from ..base import Extension
from . import core

LOG = logging.getLogger('MooseDocs.HeadingExtension')

def make_extension(**kwargs):
    return HeadingExtension(**kwargs)

def find_heading(page, id_=None):
    """Helper for returning a copy of the heading tokens."""
    return page.get('title') if id_ is None else page.get('headings', dict()).get(id_)

class HeadingExtension(Extension):
    """
    Extracts the heading from AST after tokenization.
    """
    @staticmethod
    def defaultConfig():
        config = Extension.defaultConfig()
        return config

    def initPage(self, page):
        page['title'] = None
        page['headings'] = dict()

    def postTokenize(self, page, ast):
        func = lambda n: (n.name == 'Heading')
        level = 100
        for node in moosetree.iterate(ast.root, func):
            id_ = node.get('id') or node.text('-').lower()
            node['id'] = id_
            if page['title'] is None or node['level'] < level:
                page['title'] = node.copy()
                level = node['level']
            if id_ not in page['headings']:
                page['headings'][id_] = node.copy()

    def extend(self, reader, renderer):
        self.requires(core)
