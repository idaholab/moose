#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import os
import re
import pydoc
import logging
import inspect
from MooseDocs.base import components
from MooseDocs.common import exceptions
from MooseDocs.tree import tokens, html
from . import core, command

LOG = logging.getLogger(__name__)

PyClass = tokens.newToken('PyClass')
PyFunction = tokens.newToken('PyFunction')

def make_extension(**kwargs):
    """Return the pysyntax extension object."""
    return PySyntaxExtension(**kwargs)

class PySyntax(object):
    """Helper class for extracting documentation from a python object."""
    class Info(object):
        """Data struct for storing information about a member."""
        def __init__(self, name, member):
            self.name = name
            self.internal = self.name.startswith('__') and self.name.endswith('__')
            self.private = (not self.internal) and self.name.startswith('_') and ('__' in self.name)
            self.protected = (not self.private) and (not self.internal) and self.name.startswith('_')
            self.public = not any([self.internal, self.private, self.protected])
            self.function = inspect.isfunction(member)
            self.signature = re.sub(r'self[, ]*', '', str(inspect.signature(member))) if self.function else None
            self.documentation = inspect.getdoc(member)

        def __eq__(self, rhs):
            return all([self.internal == rhs.internal,
                        self.private == rhs.private,
                        self.protected == rhs.protected,
                        self.public == rhs.public,
                        self.function == rhs.function,
                        self.signature == rhs.signature,
                        self.documentation == rhs.documentation])

        def __str__(self):
            out = '{}:\n'.format(self.name, self.signature or self.name)
            if self.documentation: out += '"{}"\n'.format(self.documentation)
            out += '  public: {}\n'.format(self.public)
            out += '  protected: {}\n'.format(self.protected)
            out += '  private: {}\n'.format(self.private)
            out += '  internal: {}\n'.format(self.internal)
            out += '  function: {}\n'.format(self.function)
            return out

    def __init__(self, cls):
        cls = pydoc.locate(cls) if isinstance(cls, str) else cls
        self.documentation = inspect.getdoc(cls)
        self.filename = inspect.getfile(cls)
        self.signature = str(inspect.signature(cls))
        self.is_class = inspect.isclass(cls)
        self.is_function = inspect.isfunction(cls)

        self._members = dict()
        if self.is_class:
            for name, member in inspect.getmembers(cls):
                self._members[name] = PySyntax.Info(name, member)
        elif self.is_function:
            name = cls.__qualname__
            self._members[name] = PySyntax.Info(name, cls)

    def items(self, function=None, **kwargs):
        """Return dict() style generator to name and `Info` objects."""
        default = False if kwargs else True
        internal = kwargs.get('internal', default)
        private = kwargs.get('private', default)
        protected = kwargs.get('protected', default)
        public = kwargs.get('public', default)

        for name, info in self._members.items():
            if any([(internal and info.internal),
                    (private and info.private),
                    (protected and info.protected),
                    (public and info.public)]) and \
                    ((function is None) or (info.function == function)):
                yield name, info

class PySyntaxExtension(command.CommandExtension):
    def extend(self, reader, renderer):
        self.requires(command)
        self.addCommand(reader, PySyntaxClassCommand())
        self.addCommand(reader, PySyntaxFunctionCommand())

        renderer.add('PyClass', RenderPyClass())
        renderer.add('PyFunction', RenderPyFunction())

class PySyntaxCommandBase(command.CommandComponent):
    COMMAND = 'pysyntax'

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['name'] = (None, "The name python object/function to extract documentation.")
        settings['heading-level'] = (2, "The heading level to use for class documentation.")
        return settings

    def _addDocumentation(self, parent, page, doc, h_level, **kwargs):
        for name, pyinfo in doc.items(**kwargs):
            h = core.Heading(parent, level=h_level, class_='moose-pysyntax-member-heading')
            fname = name + pyinfo.signature if pyinfo.signature is not None else name
            core.Monospace(core.Strong(h), string=fname)
            if pyinfo.documentation is None:
                msg = "Missing documentation for '%s'.\n%s"
                LOG.error(msg, name, doc.filename)
            else:
                self.reader.tokenize(parent, pyinfo.documentation, page)

    def _addFunctionDocumentation(self, parent, page, doc, h_level):
        sec = PyFunction(parent)
        self._addDocumentation(sec, page, doc, h_level)

    def _addClassDocumentation(self, parent, page, name, doc, h_level, **kwargs):
        """Helper for listing class members"""
        sec = PyClass(parent)

        h = core.Heading(sec, level=h_level, string=name, class_='moose-pysyntax-class-heading')
        core.Monospace(sec, string=name + doc.signature)

        if doc.documentation is None:
            msg = "Missing documentation for '%s'.\n%s"
            LOG.error(msg, name, doc.filename)
        else:
            self.reader.tokenize(sec, doc.documentation, page)

        self._addDocumentation(sec, page, doc, h_level + 1, **kwargs)

class PySyntaxClassCommand(PySyntaxCommandBase):
    SUBCOMMAND = 'class'

    @staticmethod
    def defaultSettings():
        settings = PySyntaxCommandBase.defaultSettings()
        return settings

    def createToken(self, parent, info, page, settings):
        h_level = int(settings['heading-level'])
        obj = settings.get('name', None)
        if obj is None:
            raise exceptions.MooseDocsException("The 'name' setting is required.")

        doc = PySyntax(obj)
        if not doc.is_class:
            raise exceptions.MooseDocsException("'%s' is not a python class.", obj)

        self._addClassDocumentation(parent, page, obj, doc, h_level, public=True, protected=True)
        return parent

class PySyntaxFunctionCommand(PySyntaxCommandBase):
    SUBCOMMAND = 'function'

    @staticmethod
    def defaultSettings():
        settings = PySyntaxCommandBase.defaultSettings()
        settings['heading-level'] = (2, settings['heading-level'][1])
        return settings

    def createToken(self, parent, info, page, settings):
        h_level = int(settings['heading-level'])
        obj = settings.get('name', None)
        if obj is None:
            raise exceptions.MooseDocsException("The 'name' setting is required.")

        doc = PySyntax(obj)
        if not doc.is_function:
            raise exceptions.MooseDocsException("'%s' is not a python function.", obj)

        self._addFunctionDocumentation(parent, page, doc, h_level)
        return parent

class RenderPyClass(components.RenderComponent):
    def createLatex(self, parent, token, page):
        return parent

    def createHTML(self, parent, token, page):
        return html.Tag(parent, 'div', class_='moose-pysyntax-class')

class RenderPyFunction(components.RenderComponent):
    def createLatex(self, parent, token, page):
        return parent

    def createHTML(self, parent, token, page):
        return html.Tag(parent, 'div', class_='moose-pysyntax-function')
