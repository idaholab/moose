#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import anytree
from MooseDocs.base import components
from MooseDocs.extensions import core

def make_extension(**kwargs):
    return HeadingExtension(**kwargs)

def find_heading(translator, node, bookmark=u''):
    """Helper for returning a copy of the heading tokens."""
    data = translator.getMetaData(node, 'heading')
    h = data.get(bookmark, None)
    if h:
        return h.copy()

class HeadingExtension(components.Extension):
    """
    Extracts the heading from AST after tokenization.
    """
    @staticmethod
    def defaultConfig():
        config = components.Extension.defaultConfig()
        return config

    def initMetaData(self, page, meta):
        meta.initData('heading')

    def postTokenize(self, ast, page, meta, reader):
        data = dict()
        func = lambda n: (n.name == 'Heading')
        for node in anytree.PreOrderIter(ast, filter_=func):
            id_ = node.get('id', u'')
            if id_ not in data:
                data[id_] = node.copy()

        meta.setData('heading', data)

    def extend(self, reader, renderer):
        self.requires(core)
