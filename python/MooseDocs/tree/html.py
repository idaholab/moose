#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import html
import re
import moosetree
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
        token = token or dict()
        kwargs['class'] = kwargs.pop('class_', kwargs.pop('class', token.get('class', '')))
        kwargs['style'] = kwargs.pop('style_', kwargs.pop('style', token.get('style', '')))
        kwargs['id'] = kwargs.pop('id_', kwargs.pop('id', token.get('id', '')))
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
        self['class'] = ' '.join(c).strip()

    def write(self):
        """Write the HTML as a string, e.g., <foo>...</foo>."""
        out = ''
        attr_list = []
        skip_list = ('close', 'string')
        for key, value in self.items():
            if value and (key not in skip_list):
                if isinstance(value, bool):
                    if value:
                        attr_list.append('{}'.format(key))
                else:
                    attr_list.append('{}="{}"'.format(key, str(value).strip()))

        attr = ' '.join(attr_list)
        if attr:
            out += '<{} {}>'.format(self.name, attr)
        else:
            out += '<{}>'.format(self.name)

        for child in self.children:
            out += child.write()
        if self.get('close'):
            out += '</{}>'.format(self.name)
        return out

    def text(self):
        """
        Convert String objects into a single string.
        """
        strings = []
        for node in moosetree.iterate(self):
            if node.name == 'String':
                strings.append(node['content'])
        return re.sub(r' {2,}', ' ', ' '.join(strings))

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
        kwargs.setdefault('content', '')
        kwargs.setdefault('escape', '')
        super(String, self).__init__('String', parent, **kwargs)

        if self.get('content') is None:
            self['content'] = ''

    def write(self):
        if self.get('escape'):
            return html.escape(str(self.get('content')), quote=True)
        else:
            return self.get('content')

    def copy(self, _parent=None):
        """Copy the String. These shouldn't have children, so don't worry about them."""
        return String(_parent, **self.attributes)

if __name__ == '__main__':
    import mooseutils

    class Node(object):
        def __init__(self, parent, name, **kwargs):
            self.children = list()
            self.attributes = dict()
            self.attributes.update(kwargs)
            if parent is not None:
                parent.children.append(self)

    def createTree(N, cls):
        body = cls(None, 'body')
        for i in range(N):
            div0 = cls(body, 'div')
            for j in range(N):
                div1 = cls(div0, 'div')
                for k in range(N):
                    p = cls(div1, 'p', string='STUFF')


    mooseutils.run_profile(createTree, 100, Tag)
    mooseutils.run_profile(createTree, 100, Node)
