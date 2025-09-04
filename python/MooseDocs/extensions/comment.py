# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html
import re
from ..base import components, Extension
from ..tree import tokens

def make_extension(**kwargs):
    return CommentExtension(**kwargs)

Comment = tokens.newToken('Comment', content='')

class CommentExtension(Extension):
    """
    Provides the means to make comments within markdown pages that are not rendered.
    """
    @staticmethod
    def defaultConfig():
        config = Extension.defaultConfig()
        return config

    def extend(self, reader, renderer):
        reader.addInline(CommentInline(), location='_begin')
        reader.addBlock(CommentBlock(), location='_begin')
        reader.addInline(HTMLCommentBlock(), location='_begin')

class CommentInline(components.ReaderComponent):
    """
    Inline comments begin with !! and continue to the end of a line.
    """
    RE = re.compile(r'(?<!!)!{2}(?!!)(?P<content>.*?)$', flags=re.UNICODE|re.MULTILINE)
    def createToken(self, parent, info, page, settings):
        Comment(parent, content=info['content'])
        return parent

class CommentBlock(components.ReaderComponent):
    """
    Block comments begin and end with !!! that start a line to !!! that end a line.
    """
    RE = re.compile(r'(?:\A|\n{2,})^'         # start of string or empty line
                    r'!{3}(?P<settings>.*?)$' # start of code with key=value settings
                    r'(?P<content>.*?)^!{3}'  # code and end of comment block
                    r'(?=\n*\Z|\n{2,})',      # end of string or empty line
                    flags=re.UNICODE|re.MULTILINE|re.DOTALL)

    def createToken(self, parent, info, page, settings):
        Comment(parent, content=info['content'])
        return parent

class HTMLCommentBlock(components.ReaderComponent):
    """
    Error check for HTML style comments (removed in July 2025 with idaholab/moose#29581).
    This should be removed eventually, as developers become used to the change.
    """
    RE = re.compile(r'<!--(?P<content>.*?)-->', flags=re.UNICODE|re.MULTILINE|re.DOTALL)

    def createToken(self, parent, info, page, settings):
        raise RuntimeError("Use of HTML-style comments are not allowed. Please use '!!' for inline and '!!! .... !!!' for block syntax!")
