#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import moosetree
from ..base import Extension
from . import core

def make_extension(**kwargs):
    return HeadingExtension(**kwargs)

def find_heading(page, bookmark=''):
    """Helper for returning a copy of the heading tokens."""
    data = page.get('heading', dict())
    h = data.get(bookmark, None) if data else None
    if h is not None:
        return h.copy()

class HeadingExtension(Extension):
    """
    Extracts the heading from AST after tokenization.
    """
    @staticmethod
    def defaultConfig():
        config = Extension.defaultConfig()
        return config

    def preTokenize(self, page, ast):
        page['heading'] = dict()

    def postTokenize(self, page, ast):
        func = lambda n: (n.name == 'Heading')
        for node in moosetree.iterate(ast.root, func):
            id_ = node.get('id', '')
            if id_ not in page['heading']:
                page['heading'][id_] = node.copy()

    def extend(self, reader, renderer):
        self.requires(core)
