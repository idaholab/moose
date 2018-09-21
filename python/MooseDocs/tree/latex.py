#pylint: disable=missing-docstring,no-member
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
#pylint: enable=missing-docstring
import re
from base import NodeBase, Property

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
    for key in sorted(conv.keys(), key=lambda item: - len(item)): #pylint: disable=consider-iterating-dictionary
        regex_list.append(re.escape(unicode(key)))
    regex = re.compile('|'.join(regex_list))
    return regex.sub(lambda match: conv[match.group()], text)

class Enclosure(NodeBase):
    """
    Class for enclosing other nodes in characters, e.g. [], {}.
    """
    PROPERTIES = [Property('enclose', ptype=tuple, required=True),
                  Property('string', ptype=unicode)]

    def __init__(self, *args, **kwargs):
        NodeBase.__init__(self, *args, **kwargs)
        if self.string is not None:
            String(self, content=self.string)

    def write(self):
        """
        Write LaTex as a string.
        """
        out = self.enclose[0]
        for child in self.children:
            out += child.write()
        out += self.enclose[1]
        return out

class Bracket(Enclosure):
    """
    Square bracket enclosure ([]).
    """
    def __init__(self, *args, **kwargs):
        Enclosure.__init__(self, *args, enclose=('[', ']'), **kwargs)

class Brace(Enclosure):
    """
    Curly brace enclosure ({}).
    """
    def __init__(self, *args, **kwargs):
        Enclosure.__init__(self, *args, enclose=('{', '}'), **kwargs)

class InlineMath(Enclosure):
    """
    Math enclosure ($$).
    """
    def __init__(self, *args, **kwargs):
        Enclosure.__init__(self, *args, enclose=('$', '$'), **kwargs)

class Command(NodeBase):
    """
    Typical zero or one argument command: \foo{bar}.

    If children do not exist then the braces are not included (e.g., \foo).
    """
    PROPERTIES = [Property('string', ptype=unicode),
                  Property('start', ptype=str, default=''),
                  Property('end', ptype=str, default=''),
                  Property('options', ptype=dict, default=dict())]

    def __init__(self, parent, name, *args, **kwargs):
        NodeBase.__init__(self, name=name, parent=parent, *args, **kwargs)
        if self.string is not None:
            String(self, content=self.string)

    def write(self):
        out = self.start
        out += '\\%s' % self.name
        if self.children:
            out += '{'
            for child in self.children:
                out += child.write()
            out += '}'
        out += self.end
        return out

class CustomCommand(NodeBase):
    """
    Class for building up arbitrary commands.

    Children should be Bracket or Brace objects to build up the command.
    """
    PROPERTIES = [Property('start', ptype=str, default=''),
                  Property('end', ptype=str, default='')]

    def write(self):
        """
        Write to LaTeX string.
        """
        out = self.start
        out += '\\%s' % self.name
        for child in self.children:
            out += child.write()
        out += self.end
        return out

class Environment(NodeBase):
    """
    Class for LaTeX environment: \\begin{foo}...\\end{foo}
    """
    PROPERTIES = [Property('string', ptype=unicode),
                  Property('start', ptype=str, default='\n'),
                  Property('end', ptype=str, default=''),
                  Property('after_begin', ptype=str, default='\n'),
                  Property('before_end', ptype=str, default='\n')]

    def __init__(self, parent, name, *args, **kwargs):
        NodeBase.__init__(self, name=name, parent=parent, *args, **kwargs)
        if self.string is not None:
            String(self, content=self.string)

    def write(self):
        """
        Write to LaTeX string.
        """
        out = '%s\\begin{%s}%s' % (self.start, self.name, self.after_begin)
        for child in self.children:
            out += child.write()
        out += '%s\\end{%s}%s' % (self.before_end, self.name, self.end)
        return out

class String(NodeBase):
    """
    A node for containing string content, the parent must always be a Tag.
    """
    PROPERTIES = [Property('content', default=u'', ptype=unicode),
                  Property('escape', default=True, ptype=bool)]

    def write(self):
        """
        Write to LaTeX string.
        """
        out = escape(self.content) if self.escape else self.content
        for child in self.children:
            out += child.write()
        return out
