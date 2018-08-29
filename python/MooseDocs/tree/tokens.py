#pylint: disable=missing-docstring, no-member
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import collections
import logging
import json
import mooseutils

from MooseDocs import common
from MooseDocs.common import exceptions
from MooseDocs.tree.base import Property, NodeBase

LOG = logging.getLogger(__name__)

class Token(NodeBase):
    """
    Base class for AST tokens.

    Input:
        *args, **kwarg: (Optional) All arguments and key, value pairs supplied are stored in the
                        settings property and may be retrieved via the various access methods.
    """
    PROPERTIES = [Property('recursive', default=True), # TODO: Can this go away?
                  Property('string', ptype=unicode)]
                  #Property('info')] # TODO: use property, which should work with property override

    def __init__(self, parent=None, name=None, **kwargs):
        self._info = kwargs.pop('info', None)
        super(Token, self).__init__(parent, name, **kwargs)
        self.name = self.__class__.__name__
        if self.string is not None: #pylint: disable=no-member
            String(self, content=self.string) #pylint: disable=no-member

    @property
    def info(self):
        node = self
        #pylint: disable=protected-access
        while node._info is None: # use _info to prevent infinite loop
            if node.parent is None:
                break
            node = node.parent
        return node._info

    @info.setter
    def info(self, value):
        self._info = value

    def write(self, _raw=False): #pylint: disable=arguments-differ
        """
        Return a dict() appropriate for JSON output.

        Inputs:
            _raw[bool]: An internal flag for skipping json conversion while building containers
        """
        item = collections.OrderedDict()
        item['type'] = self.__class__.__name__
        item['name'] = self.name
        item['children'] = list()

        properties = dict()
        for key, value in self._NodeBase__properties.iteritems():
            properties[key] = value
        item['properties'] = properties

        attributes = dict()
        for key, value in self._NodeBase__attributes.iteritems():
            attributes[key] = value
        item['attributes'] = attributes

        for child in self.children:
            item['children'].append(child.write(_raw=True))

        if _raw:
            return item
        return json.dumps(item, indent=2, sort_keys=True)

class CountToken(Token):
    """
    Token that maintains counts based on prefix, the Translator clears the counts prior to building.
    """
    PROPERTIES = [Property('prefix', ptype=unicode),
                  Property('number', ptype=int)]
    COUNTS = collections.defaultdict(int)
    def __init__(self, *args, **kwargs):
        Token.__init__(self, *args, **kwargs)

        if self.prefix is not None:
            CountToken.COUNTS[self.prefix] += 1
            self.number = CountToken.COUNTS[self.prefix]

class Section(Token):
    pass

class String(Token):
    """
    Base class for all tokens meant to contain characters.
    """
    PROPERTIES = [Property('content', ptype=unicode)]

class ErrorToken(Token):
    PROPERTIES = [Property('message', ptype=unicode)]

    def report(self, current):

        title = 'ERROR: {}'.format(self.message)
        filename = ''
        if current:
            source = current.source
            filename = mooseutils.colorText('{}:{}\n'.format(source, self.info.line), 'RESET')

        box = mooseutils.colorText(common.box(self.info[0], line=self.info.line, width=100),
                                   'LIGHT_CYAN')

        return u'\n{}\n{}{}\n'.format(title, filename, box)

class ExceptionToken(ErrorToken):
    """
    When the lexer object fails create a token, an error token will be created.
    """
    PROPERTIES = [Property('traceback', required=False, ptype=str)]


    def report(self, current):
        out = ErrorToken.report(self, current)
        trace = mooseutils.colorText(self.traceback, 'GREY')
        return u'{}\n{}'.format(out, trace)


class Word(String):
    """
    Letters without any spaces.
    """
    pass

class Space(String):
    """
    Space token that can define the number of space via count property.
    """
    PROPERTIES = [Property('count', ptype=int, default=1)]
    def __init__(self, *args, **kwargs):
        super(Space, self).__init__(*args, **kwargs)
        self.content = u' '

class Break(Space):
    """
    Line breaks that can define the number of breaks via count property.
    """
    def __init__(self, *args, **kwargs):
        super(Break, self).__init__(*args, **kwargs)
        self.content = u'\n'

class Punctuation(String):
    """
    Token for non-letters and non-numbers.
    """
    pass

class Number(String):
    """
    Token for numbers.
    """
    pass

class Code(Token):
    """
    Code content (i.e., Monospace content)
    """
    PROPERTIES = [Property('code', ptype=unicode, required=True),
                  Property('language', ptype=unicode, default=u'text'),
                  Property('escape', ptype=bool, default=True)]

class Heading(Token):
    """
    Section headings.
    """
    PROPERTIES = [Property('level', ptype=int)]
    def __init__(self, *args, **kwargs):
        Token.__init__(self, *args, **kwargs)

        id_ = self.get('id', None)
        if id_:
            Shortcut(self.root, key=id_, link=u'#{}'.format(id_), token=self)

class Paragraph(Token):
    """
    Paragraph token.
    """
    pass

class UnorderedList(Token):
    """
    Token for an un-ordered list (i.e., bulleted list)
    """
    pass

class OrderedList(Token):
    """
    Token for a numbered list.
    """
    PROPERTIES = [Property('start', default=1, ptype=int)]

class ListItem(Token):
    """
    List item token.
    """
    def __init__(self, *args, **kwargs):
        Token.__init__(self, *args, **kwargs)
        if not isinstance(self.parent, (OrderedList, UnorderedList)):
            raise exceptions.MooseDocsException("A 'ListItem' must have a 'OrderedList' or " \
                                                "'UnorderedList' parent.")

class Link(Token):
    """
    Token for urls.
    """
    PROPERTIES = [Property('url', required=True, ptype=unicode),
                  Property('tooltip', default=True)]

class Shortcut(Token):
    """
    Create a Shortcut that will be used to create a database of keys for use by ShortcutLink tokens.

    When creating Shortcut tokens they should be added to the root level of the tree, for example
    consider the Heading token. Also, refer to core.RenderShortcutLink for how these are used when
    rendering.

    Properties:
        key[unicode]: (Required) The shortcut key, i.e., the string used to look up content in a
                      database, the key is what is used within a ShortcutLink, the content is then
                      pulled from this token. If the 'content' and 'tokens' are empty, then the key
                      is also used for the shortcut text, see RenderShortcutLink.
        link[unicode]: (Required) The content to which the shortcut links against, e.g., the value
                       of 'href' for HTML.
    """
    PROPERTIES = [Property('key', required=True, ptype=unicode),
                  Property('link', required=True, ptype=unicode)]

class ShortcutLink(Token):
    PROPERTIES = [Property('key', ptype=unicode, required=True)]

class Monospace(Token):
    PROPERTIES = [Property('code', ptype=unicode, required=True)]

class Strong(Token):
    pass

class Emphasis(Token):
    pass

class Underline(Token):
    pass

class Strikethrough(Token):
    pass

class Quote(Token):
    pass

class Superscript(Token):
    pass

class Subscript(Token):
    pass

class Label(Token):
    PROPERTIES = [Property('text', required=True, ptype=unicode)]

class Float(Token):
    PROPERTIES = [Property('id', ptype=str),
                  Property('caption', ptype=unicode),
                  Property('label', ptype=str, required=True)]
