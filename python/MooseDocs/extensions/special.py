#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import re

from ..base import components, Extension
from ..tree import tokens

def make_extension(**kwargs):
    """
    Create and return the CoreExtension object for converting from markdown to html/latex.
    """
    return SpecialExtension(**kwargs)

class SpecialExtension(Extension):
    @staticmethod
    def defaultConfig():
        """CoreExtension configuration options."""
        config = Extension.defaultConfig()
        return config

    def extend(self, reader, renderer):
        reader.addInline(HTMLNumberCode(), location='<PunctuationInline')
        reader.addInline(HTMLEntityCode(), location='<PunctuationInline')

class HTMLNumberCode(components.ReaderComponent):
    RE = re.compile(r'(?P<code>&#[0-9]+;)')
    def createToken(self, parent, info, page, settings):
        tokens.String(parent, content=info['code'], escape=False)
        return parent

class HTMLEntityCode(components.ReaderComponent):
    RE = re.compile(r'(?P<code>&[A-Za-z0-9]+;)')
    def createToken(self, parent, info, page, settings):
        tokens.String(parent, content=info['code'], escape=False)
        return parent
