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

        style = kwargs.pop('style', None)
        self.__style = dict()
        if style:
            for pair in style.split(';'):
                if pair:
                    key, value = pair.split(':')
                    self.__style[key.strip()] = value.strip()

        super(Tag, self).__init__(name=name, parent=parent, **kwargs)

        #pylint: disable=no-member
        if self.string:
            String(self, content=self.string)

    def __setitem__(self, key, value):
        """
        Create/set an attribute.
        """
        if (key == 'class') and (key in self):
            NodeBase.__setitem__(self, key, '{} {}'.format(self.get(key), value))
        else:
            NodeBase.__setitem__(self, key, value)

    @property
    def style(self):
        """Return the HTML style attribute."""
        return self.__style

    def write(self):
        """Write the HTML as a string, e.g., <foo>...</foo>."""
        out = ''
        attr_list = []
        for key, value in self.attributes.iteritems():
            if value:
                attr_list.append('{}="{}"'.format(key, str(value).strip()))

        if self.__style:
            style_list = []
            for key, value in self.__style.iteritems():
                if value:
                    style_list.append('{}:{}'.format(key, str(value).strip()))
            style_string = ';'.join(style_list)
            if not style_string.endswith(';'):
                style_string += ';'
            attr_list.append('style="{}"'.format(style_string))

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
