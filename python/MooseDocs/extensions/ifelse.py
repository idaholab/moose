#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import os
import sys
import re
import importlib
import collections
import logging
import moosetree
import mooseutils
import MooseDocs
from ..base import Extension, components
from ..base.readers import MarkdownReader
from ..common import exceptions
from ..tree import tokens
from TestHarness import util
from . import command, appsyntax

LOG = logging.getLogger(__name__)

# A token for keeping track of if/elif/else statements. If the token has children than the statement
# is True and the content within the token should be tokenized and displayed
Statement = tokens.newToken('Statement')
Condition = tokens.newToken('Condition', command=None, content=None, function=None)

def make_extension(**kwargs):
    return IfElseExtension(**kwargs)

def hasMooseApp(ext, app):
    """Module function for searching for the existence of a registered application name."""
    return ext.hasRegistredApp(app)

def hasSubmodule(ext, name):
    """Module function for testing if an application has a submodule ending with the given name."""
    return ext.hasSubmodule(name)

def hasLibtorch(ext):
    """Module function for testing if an application was compiled with libtorch."""
    return ext.hasConfigOption('libtorch', 'true')

def hasPage(ext, filename):
    """Module function for the existence of markdown page."""
    return ext.translator.findPage(filename, throw_on_zero=False) is not None

class IfElseExtension(command.CommandExtension):
    """
    Allows the if/elif/else statements to control content.
    """
    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        config['modules'] = (list(), "A list of python modules to search for functions; by default the 'ifelse.py' extension is included. All functions called must accept the extension as the first argument.")
        return config

    def __init__(self, *args, **kwargs):
        command.CommandExtension.__init__(self, *args, **kwargs)

        # List of registered apps, see preExecute
        self._registerd_apps = set()
        self._current_app = None

        # Build list of modules for function searching and include this file by default
        self._modules = list()
        self._modules.append(sys.modules[__name__])
        for name in self.get('modules'):
            try:
                self._modules.append(importlib.import_module(name))
            except ImportError as e:
                msg = "Failed to import the supplied '{}' module.\n{}"
                raise exceptions.MooseDocsException(msg, name, e)

    def preExecute(self):
        """Populate a list of registered applications."""

        syntax = None
        for ext in self.translator.extensions:
            if isinstance(ext, appsyntax.AppSyntaxExtension):
                syntax = ext.syntax
                break

        if syntax is not None:
            for node in moosetree.iterate(syntax):
                self._registerd_apps.update(node.groups())

    def hasRegistredApp(self, name):
        """Helper for the 'hasMooseApp' function."""
        if not self._registerd_apps:
            msg = "The 'hasMooseApp' function requires the 'appsyntax' extension to have complete syntax, the 'ifelse' extension is being disabled."
            self.setActive(False)
            LOG.warning(msg)

        return name in self._registerd_apps

    def hasSubmodule(self, name):
        """Helper for the 'hasSubmodule' function."""
        status = mooseutils.git_submodule_info(MooseDocs.ROOT_DIR, '--recursive')
        return any([repo.endswith(name) for repo in status.keys()])

    def hasConfigOption(self, option, value):
        moose_dir = os.getenv('MOOSE_DIR',
                              os.path.abspath(os.path.join(os.path.dirname(os.path.realpath(__file__)), '..', '..', '..')))

        # getMooseconfig returns a set of ('ALL', value ), so we have to iterate through it
        for it in util.getMooseConfigOption(moose_dir, option):
            if it.lower() == value.lower():
                return True

        return False

    def extend(self, reader, renderer):
        self.requires(command)
        self.addCommand(reader, IfCommand())
        self.addCommand(reader, ElifCommand())
        self.addCommand(reader, ElseCommand())

    def getFunction(self, func_name):
        """Find a function in the list of loaded modules."""
        for mod in self._modules:
            func = getattr(mod, func_name, None)
            if func is not None:
                return func

        msg = "Unable to locate function '{}' in the listed modules, the loaded modules include:\n{}"
        raise exceptions.MooseDocsException(msg, func_name, '    \n'.join([m.__name__ for m in self._modules]))

class IfCommandBase(command.CommandComponent):
    """
    Base for if/elif commands that require the evaluation of a function
    """
    SUBCOMMAND = None
    FUNCTION_RE = re.compile(r'(?P<not>!*)(?P<function>\w+)(?P<args>\(.*?\))$', flags=re.MULTILINE|re.UNICODE)

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['function'] = (None, "The function---with arguments---to evaluate. This setting is +required+.")
        return settings

    def createTokenHelper(self, parent, info, page, settings):
        group = MarkdownReader.INLINE if MarkdownReader.INLINE in info else MarkdownReader.BLOCK
        command = info['command']
        function = settings['function']

        # Must supply 'function'
        if function is None:
            msg = "The 'function' setting is required."
            raise exceptions.MooseDocsException(msg)

        # 'if' creates a statement that contains Condition tokens
        if command == 'if':
            parent = Statement(parent)
        elif parent.children:
            parent = parent.children[-1]

        if parent.name != 'Statement':
            msg = "The 'Condition' being created is out of place, it must be in sequence with an " \
                  "an 'if' and 'elif' condition(s)."
            raise exceptions.MooseDocsException(msg)

        condition = Condition(parent, command=command, content=info[group], function=function)
        return condition, group

    def evaluateFunction(self, settings):
        """Helper for evaluating the 'function' setting."""

        # Separate function name from arguments
        function = settings['function']
        match = IfCommand.FUNCTION_RE.search(function)
        if match is None:
            msg = "Invalid expression for 'function' setting: {}"
            raise exceptions.MooseDocsException(msg, function)

        # Locate and evaluate the function
        func = self.extension.getFunction(match.group('function'))

        # If we don't haver agrs we define an empty tupple, otherwise we
        # include every argument (which are treated as strings here), the trailing
        # ',' is to always create tuple
        arg_str = "tuple()" if match.group('args') == "()" else match.group('args')[:-1] + ',)'
        args = eval(arg_str)
        value = func(self.extension, *args)

        # Require that an actual 'bool' is returned to avoid un-intended operation, for example
        # if a function is returned it would evaluate to True.
        if not isinstance(value, bool):
            msg = "The return value from the function '{}' must be a 'bool' type, but '{}' returned."
            raise exceptions.MooseDocsException(msg, match.group('function'), type(value))

        return not value if match.group('not') == '!' else value

class IfCommand(IfCommandBase):
    COMMAND = 'if'

    def createToken(self, parent, info, page, settings):
        condition, group = IfCommandBase.createTokenHelper(self, parent, info, page, settings)

        # If the condition is not met, then remove content by setting it to None
        if not self.evaluateFunction(settings):
            info._LexerInformation__match[group] = None

        return condition

class ElifCommand(IfCommandBase):
    COMMAND = 'elif'

    def createToken(self, parent, info, page, settings):
        condition, group = IfCommandBase.createTokenHelper(self, parent, info, page, settings)

        # Condition has already been satisfied if any sibling has content
        satisfied = any(bool(c.children) for c in condition.siblings)

        # If a previous condition is met or this condition is not met, remove content
        if satisfied or not self.evaluateFunction(settings):
            info._LexerInformation__match[group] = None

        return condition

class ElseCommand(command.CommandComponent):
    COMMAND = 'else'
    SUBCOMMAND = None

    def createToken(self, parent, info, page, settings):
        group = MarkdownReader.INLINE if MarkdownReader.INLINE in info else MarkdownReader.BLOCK

        statement = parent.children[-1] if len(parent) > 0 else None
        prev = statement.children[-1] if (statement is not None and len(statement) > 0) else None
        if prev is None:
            msg = "The 'else' command must follow an 'if' or 'elif' condition."
            raise exceptions.MooseDocsException(msg)

        # Condition has already been satisfied if any sibling has content
        condition = Condition(parent.children[-1], command=info['command'], content=info[group])
        satisfied = any(bool(c.children) for c in condition.siblings)
        if satisfied:
            info._LexerInformation__match[group] = None

        return condition
