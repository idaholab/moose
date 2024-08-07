#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

"""
Module for defining the default Lexer objects that plugin to base.Reader objects.
"""
import collections
import logging
import traceback
import types
import re

from .. import common
from ..tree import tokens

LOG = logging.getLogger(__name__)

class Pattern(object):
    """
    Object for storing token creation items.

    Inputs:
         name[str]: The name of the tokenization component (e.g., Heading).
         regex: Compiled python re for creating token.
         function: The function to call when a match occurs (components.Component.__call__).
    """
    def __init__(self, name, regex, function):
        self.name = name
        self.regex = regex
        self.function = function

    def __str__(self):
        return '{}: re={}; func={}'.format(self.name, self.regex, self.function)

class Grammar(object):
    """
    Defines a generic Grammar that contains the Token objects necessary to build an
    abstract syntax tree (AST). This class defines the order that the tokens will be
    applied to the Lexer object and the associated regular expression that define the
    text associated with the Token object.
    """
    def __init__(self):
        self.__patterns = common.Storage(Pattern)

    def add(self, name, regex, function, location='_end'):
        """
        Method for adding a Token definition to the Grammar object.

        Inputs:
            name[str]: The name of the grammar definition, this is utilized for
                       ordering of the definitions.
            regex[re]: A compiled re object that defines what text the token should
                       be associated.
            function[function]: A function that accepts a regex match object as input and
                                returns a token object.
            location[int or str]:  (Optional) In the case of an int type, this is an
                                   index indicating the location in the list of definitions
                                   to insert. In the case of a str type the following syntax
                                   is support to insert definitions relative to other
                                   definitions.
                                        '_begin': Insert the new definition at the beginning
                                                  of the list of definitions, this is the same
                                                  as using an index of 0.
                                        '_end': Append the new definition at the end of the list
                                                of definitions (this is the default).
                                        '<foo': Insert the new definition before the definition
                                                named 'foo'.
                                        '>foo': Insert the new definition after the definition
                                                named 'foo'.
        """
        # Add the supplied information to the storage.
        self.__patterns.add(name, Pattern(name, regex, function), location)

    def __contains__(self, key):
        """
        Return True if the key is contained in the defined patterns.
        """
        return key in self.__patterns

    def __getitem__(self, key):
        """
        Return the pattern for a given key.
        """
        return self.__patterns[key]

    def __iter__(self):
        """
        Provide iterator access to the patterns.
        """
        for obj in self.__patterns:
            yield obj

    def __str__(self):
        out = []
        for obj in self.__patterns:
            out.append(str(obj))
        return 'Grammar:\n' + '\n'.join(out)

class LexerInformation(object):
    """
    Lexer meta data object to keep track of necessary information for strong error reporting.

    Inputs:
        match[re.Match]: The regex match object from which a Token object is to be created.
        pattern[Grammar.Pattern]: Grammar pattern definition, see Grammar.py.
        line[int]: Current line number in supplied parsed text.
    """
    def __init__(self, match=None, pattern=None, line=None):
        self.__match = dict()
        self.__pattern = pattern.name
        self.__line = line

        self.__match[0] = match.group(0)
        for i, group in enumerate(match.groups()):
            self.__match[i+1] = group
        for key, value in match.groupdict().items():
            self.__match[key] = value

    @property
    def line(self):
        """
        Return the line number for the regex match.
        """
        return self.__line

    @property
    def pattern(self):
        """
        Return the Grammar.Pattern for the regex match.
        """
        return self.__pattern

    @property
    def match(self):
        """
        Return the re Match object.
        """
        return self.__match

    def __getitem__(self, value):
        """
        Return the regex group by number or name.

        Inputs:
            value[int|str]: The regex group index or name.
        """
        return self.__match[value]

    def get(self, name, default=None):
        """
        Return the group or the supplied default.
        """
        return self.__match.get(name, default)

    def keys(self):
        """
        List of named regex groups.
        """
        return self.__match.keys()

    def items(self):
        """
        Iterate over the named groups.
        """
        for key, value in self.__match.items():
            yield key, value

    def __contains__(self, value):
        """
        Check if a named group exists in the regex match.
        """
        return value in self.__match

    def __str__(self):
        """
        Return a reasonable string for debugging.
        """
        return 'line:{} match:{} pattern:{}'.format(self.__line, self.__match, self.__pattern)

class Lexer(object):
    """
    Simple regex base lexer.

    This provides a basic linear means to use regular expressions to tokenize text. The tokenize
    method starts with the complete text, loops through all the patterns (defined in Grammar
    object). When a match is found the function attached to the grammar is called. The text is then
    searched again starting at the end position of the last match.

    Generally, this object should not be used. It is designed to provide the general capability
    needed for the RecursiveLexer.
    """
    def __init__(self):
        pass

    def tokenize(self, parent, text, page, grammar, line=1):
        """
        Perform tokenization of the supplied text.

        Inputs:
            parent[tree.tokens]: The parent token to which the new token(s) should be attached.
            grammar[Grammar]: Object containing the grammar (defined by regexs) to search.
            text[str]: The text to tokenize.
            line[int]: The line number to start with, this allows for nested calls to begin with
                       the correct line.

        NOTE: If the functions attached to the Grammar object raise an Exception it will
              be caught by this object and converted into an Exception token. This allows for
              the entire text to be tokenized and have the errors report upon completion.
        """
        if not isinstance(text, str):
            msg = "EXCEPTION: {}:{}\n{}".format(page.source, line,
                                                "The supplied text must be str.")
            raise TypeError(msg)

        n = len(text)
        pos = 0
        while pos < n:
            match = None
            for pattern in grammar:
                match = pattern.regex.match(text, pos)
                if match:
                    info = LexerInformation(match, pattern, line)
                    try:
                        obj = self.buildToken(parent, pattern, info, page)
                    except Exception as e:
                        obj = tokens.ErrorToken(parent,
                                                message=str(e),
                                                traceback=traceback.format_exc())

                    if obj is not None:
                        obj.info = info
                        line += match.group(0).count('\n')
                        pos = match.end()

                        break
                    else:
                        continue

            if match is None:
                break

        # Produce Exception token if text remains that was not matched
        if pos < n:
            msg = 'Unprocessed text exists.'
            tokens.ErrorToken(parent, message=msg)

    def buildToken(self, parent, pattern, info, page):
        """
        Return a token object for the given lexer information.
        """
        return pattern.function(parent, info, page)

class RecursiveLexer(Lexer):
    """
    A lexer that accepts multiple grammars and automatically processes the content recursively
    base on regex group names.

    Inputs:
        base[str]: The starting (or base) grammar group name to begin tokenization.
        *args: Additional grammar group names that will be included.

    Example:
       Given a regular expression such as '(?P<foo>.*>)'. If this object has a 'grammar' object
       defined with the name 'foo', tokenize will automatically be called with the content
       of the 'foo' group using the 'foo' grammar.
    """
    def __init__(self, base, *args):
        Lexer.__init__(self)
        self._grammars = collections.OrderedDict()
        self._grammars[base] = Grammar()
        for name in args:
            self._grammars[name] = Grammar()

    def grammar(self, group=None):
        """
        Return the Grammar object by group name.

        Inputs:
            group[str]: The name of the grammar group to return, if not given the base is returned.
        """
        if group is None:
            group = list(self._grammars.keys())[0]
        return self._grammars[group]

    def grammars(self):
        """
        Return the complete dictionary of Grammar objects.
        """
        return self._grammars

    def add(self, group, *args):
        """
        Append a component to the provided group.
        """
        self.grammar(group).add(*args)

    def buildToken(self, parent, pattern, info, page):
        """
        Override the Lexer.buildToken method to recursively tokenize base on group names.
        """
        obj = super(RecursiveLexer, self).buildToken(parent, pattern, info, page)

        if (obj is not None) and (obj is not parent) and obj.get('recursive'):
            for key, grammar in self._grammars.items():
                if key in info.keys():
                    text = info[key]
                    if text is not None:
                        self.tokenize(obj, text, page, grammar, info.line)
        return obj
