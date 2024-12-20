#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import logging
import copy
import moosetree
import MooseDocs
from .base import NodeBase

LOG = logging.getLogger(__name__)


def newToken(name, parent=None, **defaults):
    """
    Function for creating Token objects with unique names and default attributes.

    TODO: Add a default system that has type checking and required checking (only in DEBUG)
    """
    if MooseDocs.LOG_LEVEL == logging.DEBUG:
        pass # Future consistency checking

    if parent is not None:
        old = defaults
        defaults = parent(None).attributes
        defaults.update(old)

    def tokenGenerator(parent, **kwargs):
        if MooseDocs.LOG_LEVEL == logging.DEBUG:
            pass # Future consistency checking
        inputs = copy.copy(defaults)
        inputs.update(**kwargs)
        return Token(name, parent, **inputs)

    return tokenGenerator

class Token(NodeBase):
    """
    Base class for AST tokens. All tokens are of this type, but should be generated with the
    newToken function to assign default attributes.

    Input:
        *args, **kwarg: (Optional) All arguments and key, value pairs supplied are stored in the
                        settings property and may be retrieved via the various access methods.
    """
    def __init__(self, name=None, parent=None, **kwargs):
        for key in list(kwargs):
            if key.endswith('_'):
                kwargs[key[:-1]] = kwargs.pop(key)
        kwargs.setdefault('recursive', True)
        kwargs.setdefault('string', None)
        super(Token, self).__init__(name, parent, **kwargs)

        # Storage for Lexer Information object
        self._info = None

        # Create string on demand
        string = self.attributes.pop('string', None)
        if string is not None:
            String(self, content=string)

    @property
    def info(self):
        """Retrieve the Information object from a parent node."""
        node = self
        # use _info to prevent infinite loop
        while node._info is None and not node.is_root:
            node = node.parent
        return node._info

    @info.setter
    def info(self, value):
        self._info = value

    def text(self, sep=' '):
        """
        Convert String objects into a single string.
        """
        strings = []
        for node in moosetree.iterate(self):
            if node.name in ['Word', 'Number', 'String']:
                strings.append(node['content'])
        return sep.join(strings)

    def copy(self, _parent=None, info=False):
        """
        Create a copy of this node. This returns an equivalent root node when '_parent=None'. If
        'info=True', then the info property of the original token is preserved in the copy.
        """
        tok = Token(self.name, _parent, **self.attributes)
        tok.info = self.info if info else None
        for child in self.children:
            child.copy(_parent=tok)
        return tok

    def copyToToken(self, token):
        """Copy the children from this token to the supplied parent."""
        for child in self.copy().children:
            child.parent = token

    def toDict(self):
        """Convert tree into a dict."""
        return dict(name=self.name,
                    attributes=self.attributes,
                    children=[child.toDict() for child in self.children])

String = newToken('String', content='')
ErrorToken = newToken('ErrorToken', message='', traceback=None)
DisabledToken = newToken('DisabledToken')
