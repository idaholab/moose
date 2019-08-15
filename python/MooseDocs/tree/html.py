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
from .base import NodeBase

class Tag(NodeBase):
    """
    A node representing an HTML tag (e.g., h1, strong).

    Inputs:
        parent[Tag]: The parent html Tag node.
        name[str]: The tag name (e.g., h1 or span).
        token[Token]: The token from which the style, id, and class shall be
                      extracted.
        kwargs: Key, value pairs to be added to the html attributes, if
                'class_', 'id_', or 'style' are provided the override those
                values within the token (if provided).
    """
    def __init__(self, parent=None, name=None, token=None, **kwargs):
        kwargs.setdefault('close', True)
        kwargs.setdefault('string', None)
        if token is not None:
            kwargs['class'] = kwargs.pop('class_', kwargs.pop('class', token.get('class', u'')))
            kwargs['style'] = kwargs.pop('style_', kwargs.pop('style', token.get('style', u'')))
            kwargs['id'] = kwargs.pop('id_', kwargs.pop('id', token.get('id', u'')))
        else:
            kwargs['class'] = kwargs.pop('class_', kwargs.pop('class', u''))
            kwargs['style'] = kwargs.pop('style_', kwargs.pop('style', u''))
            kwargs['id'] = kwargs.pop('id_', kwargs.pop('id', u''))
        super(Tag, self).__init__(name=name, parent=parent, **kwargs)

        string = self.attributes.pop('string', None)
        if string is not None:
            String(self, content=string)

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
        c = self.get('class', '').strip().split(' ')
        c += [a.strip() for a in args]
        self.set('class', ' '.join(c).strip())

    def write(self):
        """Write the HTML as a string, e.g., <foo>...</foo>."""
        out = ''
        attr_list = []
        skip_list = ('close', 'string')
        for key, value in self.iteritems():
            if value and (key not in skip_list):
                attr_list.append(u'{}="{}"'.format(key, str(value).strip()))

        attr = u' '.join(attr_list)
        if attr:
            out += u'<{} {}>'.format(self.name, attr)
        else:
            out += u'<{}>'.format(self.name)

        for child in self.children:
            out += child.write()
        if self.get('close'): #pylint: disable=no-member
            out += u'</{}>'.format(self.name)
        return out

    def text(self):
        """
        Convert String objects into a single string.
        """
        strings = []
        for node in anytree.PreOrderIter(self):
            if node.name == 'String':
                strings.append(node['content'])
        return u' '.join(strings)

    def copy(self, _parent=None):
        """Copy the tree from this node."""
        root = Tag(_parent, self.name, **self.attributes)
        for child in self.children:
            child.copy(_parent=root)
        return root

class String(NodeBase):
    """
    A node for containing string content.
    """
    def __init__(self, parent=None, **kwargs):
        kwargs.setdefault('content', u'')
        kwargs.setdefault('escape', u'')
        super(String, self).__init__('String', parent, **kwargs)

        if self.get('content') is None:
            self.set('content', u'')

    def write(self):
        if self.get('escape'):
            return cgi.escape(str(self.get('content')), quote=True)
        else:
            return self.get('content')

    def copy(self, _parent=None):
        """Copy the String. These shouldn't have children, so don't worry about them."""
        return String(_parent, **self.attributes)
