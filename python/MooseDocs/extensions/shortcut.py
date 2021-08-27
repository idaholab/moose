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
from ..common import report_error
from . import core

LOG = logging.getLogger(__name__)

def make_extension(**kwargs):
    return ShortcutExtension(**kwargs)

def find_shortcut(page, key):
    """Helper for returning a shortcut token with the specified key."""
    return page.get('shortcuts', dict()).get(key)

class ShortcutExtension(Extension):
    """
    Extracts the shortcuts from page abstract syntax trees prior to rendering.
    """
    @staticmethod
    def defaultConfig():
        config = Extension.defaultConfig()
        return config

    def extend(self, reader, renderer):
        self.requires(core) # shortcut tokens are created by the Core extension

    def initPage(self, page):
        page['shortcuts'] = dict()

    def preRender(self, page, ast, result):
        """Cache Shortcut tokens in the page attributes."""

        page['shortcuts'].clear() # in case this page is being live reloaded
        for node in moosetree.findall(ast, lambda n: (n.name == 'Shortcut')):
            key = node['key']
            if key not in page['shortcuts']:
                page['shortcuts'][key] = node.copy();
            else:
                msg = "The shortcut link key '{}' has already been used."
                LOG.error(report_error(msg.format(key), page.source,
                                       node.info.line if node.info else None,
                                       node.info[0] if node.info and node.info[0].strip() else ''))
