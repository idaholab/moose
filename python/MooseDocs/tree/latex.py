#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import re
import copy
from ..common import exceptions
from .base import NodeBase

def parse_style(token):
    """Helper for converting style entries into a dict."""
    style = dict()
    for match in re.finditer(r'(?P<key>\S+?)\s*:\s*(?P<value>.*?)(?:;|\Z)', token.get('style', '')):
        style[match.group('key')] = match.group('value').strip()
    return style

def escape(text):
    """
    Escape LaTeX commands.

    Inputs:
        text: a plain text message
    """
    conv = {
        '&': '\\&',
        '%': '\\%',
        '$': '\\$',
        '#': '\\#',
        '_': '\\_',
        '{': '\\{',
        '}': '\\}',
        '^': '{\\textasciicircum}',
        '~': '{\\textasciitilde}',
        '\\': '{\\textbackslash}',
        '<': '{\\textless}',
        '>': '{\\textgreater}',
    }

    regex_list = []
    for key in sorted(conv.keys(), key=lambda item: - len(item)):
        regex_list.append(re.escape(str(key)))
    regex = re.compile('|'.join(regex_list))
    return regex.sub(lambda match: conv[match.group()], text)

class LatexBase(NodeBase):
    """Base class for Latex nodes."""
    def __init__(self, *args, **kwargs):
        string = kwargs.pop('string', None)
        kwargs.setdefault('info', None)
        NodeBase.__init__(self, *args, **kwargs)

        if string is not None:
            String(self, content=string, escape=kwargs.get('escape', True))

    def copy(self):
        """Creates copy of the Node"""
        return copy.copy(self)

class EnclosureBase(LatexBase):
    """
    Class for enclosing other nodes in characters, e.g. [], {}.
    """
    def __init__(self, *args, **kwargs):
        LatexBase.__init__(self, *args, **kwargs)

        if self.get('enclose', None) is None:
            raise exceptions.MooseDocsException("The 'enclose' property is required.")

    def write(self):
        """
        Write LaTeX as a string.
        """
        enclose = self.get('enclose')
        out = enclose[0]
        for child in self.children:
            out += child.write()
        out += enclose[1]
        return out

class Bracket(EnclosureBase):
    """
    Square bracket enclosure ([]).
    """
    def __init__(self, parent=None, **kwargs):
        EnclosureBase.__init__(self, 'Bracket', parent, enclose=('[', ']'), **kwargs)

class Brace(EnclosureBase):
    """
    Curly brace enclosure ({}).
    """
    def __init__(self, parent=None, **kwargs):
        EnclosureBase.__init__(self, 'Brace', parent, enclose=('{', '}'), **kwargs)

class InlineMath(EnclosureBase):
    """
    Math enclosure ($$).
    """
    def __init__(self, parent=None, **kwargs):
        EnclosureBase.__init__(self, 'InlineMath', parent, enclose=('$', '$'), **kwargs)

class Command(LatexBase):
    """
    Typical zero or one argument command: \foo{bar}.

    If children do not exist then the braces are not included (e.g., \foo).
    """
    def __init__(self, parent, name, **kwargs):
        kwargs.setdefault('start', '')
        kwargs.setdefault('end', '')
        kwargs.setdefault('args', [])
        kwargs.setdefault('optional', False)
        LatexBase.__init__(self, name, parent, **kwargs)

    def write(self):
        optional = self.get('optional', False)
        out = self.get('start')
        out += '\\%s' % self.name
        for arg in self.get('args'):
            out += arg.write()
        if self.children:
            out += '[' if optional else '{'
            for child in self.children:
                out += child.write()
            out += ']' if optional else '}'
        out += self.get('end')
        return out

class Environment(LatexBase):
    """
    Class for LaTeX environment: \\begin{foo}...\\end{foo}
    """
    def __init__(self, parent, name, **kwargs):
        kwargs.setdefault('start', '\n')
        kwargs.setdefault('end', '\n')
        kwargs.setdefault('args', [])
        kwargs.setdefault('after_begin', '\n')
        kwargs.setdefault('before_end', '\n')
        LatexBase.__init__(self, name, parent, **kwargs)

    def write(self):
        """
        Write to LaTeX string.
        """
        out = '%s\\begin{%s}' % (self.get('start'), self.name)
        for arg in self.get('args'):
            out += arg.write()
        out += self.get('after_begin')
        for child in self.children:
            out += child.write()
        out += '%s\\end{%s}%s' % (self.get('before_end'), self.name, self.get('end'))
        return out

class String(NodeBase):
    """
    A node for containing string content, the parent must always be a Tag.
    """
    def __init__(self, parent=None, **kwargs):
        kwargs.setdefault('content', '')
        kwargs.setdefault('escape', True)
        NodeBase.__init__(self, 'String', parent, **kwargs)

    def write(self):
        """
        Write to LaTeX string.
        """
        out = escape(self.get('content')) if self.get('escape') else self.get('content')
        for child in self.children:
            out += child.write()
        return out

def create_settings(*args, **kwargs):
    """Creates token with key, value pairs settings application."""
    args = list(args)
    args += ["{}={}".format(k, v) for k, v in kwargs.items()]
    opt = Bracket(None, escape=False, string=",".join(args))
    return opt
