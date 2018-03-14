#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
#pylint: enable=missing-docstring
import cgi
import anytree
from base import NodeBase, Property

class Tag(NodeBase):
    """
    A node representing an HTML tag (e.g., h1, strong).
    """
    PROPERTIES = [Property('close', default=True, ptype=bool), Property('string', ptype=unicode)]

    def __init__(self, parent, name, **kwargs):

        super(Tag, self).__init__(name=name, parent=parent, **kwargs)

        #pylint: disable=no-member
        if self.string:
            String(self, content=self.string)

    def addStyle(self, style):
        """
        Add to the existing style settings.
        """
        s = self.get('style', '').split(';')
        s += style.split(';')
        self['style'] = ';'.join(s)

    def addClass(self, *args):
        """
        Add to the existing class list.
        """
        c = self.get('class', '').split(' ')
        c += args
        self['class'] = ' '.join(c)

    def write(self):
        """Write the HTML as a string, e.g., <foo>...</foo>."""
        out = ''
        attr_list = []
        for key, value in self.attributes.iteritems():
            if value:# and (key != 'class'):
                attr_list.append('{}="{}"'.format(key, str(value).strip()))

        attr = ' '.join(attr_list)
        if attr:
            out += '<{} {}>'.format(self.name, attr)
        else:
            out += '<{}>'.format(self.name)

        for child in self.children:
            out += child.write()
        if self.close: #pylint: disable=no-member
            out += '</{}>'.format(self.name)
        return out

    def text(self):
        """
        Convert String objects into a single string.
        """
        strings = []
        for node in anytree.PreOrderIter(self):
            if isinstance(node, String) and not node.hide:
                strings.append(node.content)
        return u' '.join(strings)

class String(NodeBase):
    """
    A node for containing string content, the parent must always be a Tag.
    """
    PROPERTIES = [Property('content', default=u'', ptype=unicode),
                  Property('escape', default=False, ptype=bool),
                  Property('hide', default=False, ptype=bool)]

    def __init__(self, parent=None, **kwargs):
        super(String, self).__init__(parent=parent, **kwargs)

        if (self.parent is not None) and (not isinstance(self.parent, Tag)):
            msg = "If set, the parent of he html.String '{}' must be a html.Tag object, a '{}' " \
                  " was provided."
            raise TypeError(msg.format(self.name, type(self.parent).__name__))

    def write(self):
        if self.escape: #pylint: disable=no-member
            return cgi.escape(self.content, quote=True) #pylint: disable=no-member
        else:
            return self.content #pylint: disable=no-member
