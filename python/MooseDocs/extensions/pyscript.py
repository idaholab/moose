#pylint: disable=missing-docstring
import re
import os
from MooseDocs import common
from MooseDocs.common import exceptions
import chigger
import MooseDocs
from MooseDocs.extensions import command, table, floats
from MooseDocs.tree import tokens


def make_extension(**kwargs):
    return PyScriptExtension(**kwargs)

class PyScriptExtension(command.CommandExtension):
    """
    Enables the direct use of MooseDocs markdown within a python script.
    """

    def extend(self, reader, renderer):
        self.requires(command, table, floats)
        self.addCommand(ChiggerKeybindings())
        self.addCommand(ChiggerOptions())
        self.addCommand(ChiggerTests())


class ChiggerKeybindings(command.CommandComponent):
    COMMAND = 'chigger'
    SUBCOMMAND = 'keybindings'

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['caption'] = (None, "The caption to use for the listing content.")
        settings['object'] = (None, "The chigger object to load.")
        settings['prefix'] = (u'Table', "Text to include prior to the included text.")
        return settings

    def createToken(self, info, parent):
        obj = eval(self.settings['object'])
        rows = []
        for key, bindings in obj.validKeyBindings().bindings.iteritems():
            txt = 'shift-{}'.format(key[0]) if key[1] else key[0]
            for i, binding in enumerate(bindings):
                if i == 0:
                    rows.append((txt, binding.description))
                else:
                    rows.append(('', binding.description))

        if rows:
            tokens.Heading(parent, level=2, string=u"Available Keybindings")
            tbl = table.builder(rows, headings=['Binding', 'Description'])
            tbl.parent = floats.create_float(parent, self.extension, self.settings, **self.attributes)

        return parent

class ChiggerOptions(command.CommandComponent):
    COMMAND = 'chigger'
    SUBCOMMAND = 'options'

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['caption'] = (None, "The caption to use for the listing content.")
        settings['object'] = (None, "The chigger object to load.")
        settings['prefix'] = (u'Table', "Text to include prior to the included text.")
        return settings

    def createToken(self, info, parent):
        obj = eval(self.settings['object'])
        rows = []
        for option in obj.validOptions().itervalues():
            rows.append((option.name,
                         repr(option.default) if option.default is not None else '',
                         option.vtype.__name__ if option.vtype is not None else '',
                         option.size if option.size is not None else '',
                         repr(option.allow) if option.allow is not None else '',
                         option.doc))

        if rows:
            tokens.Heading(parent, level=2, string=u"Available Options")
            tbl = table.builder(rows, headings=['Name', 'Default', 'Type', 'Size', 'Allowed', 'Description'])
            tbl.parent = floats.create_float(parent, self.extension, self.settings, **self.attributes)
        return parent

class ChiggerTests(command.CommandComponent):
    COMMAND = 'chigger'
    SUBCOMMAND = 'tests'

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['object'] = (None, "The chigger object to load.")
        settings['location'] = (os.path.join('python', 'chigger', 'tests'),
                                "Location to search for python tests.")
        return settings

    def createToken(self, info, parent):

        location = os.path.join(MooseDocs.MOOSE_DIR, self.settings['location'])
        obj = self.settings['object']

        ul = tokens.UnorderedList(None, class_='moose-list-chigger-tests')
        for root, _, files in os.walk(location):
            for name in filter(lambda n: n.endswith('.py'), files):
                filename = os.path.join(root, name)
                local = unicode(filename.replace(MooseDocs.ROOT_DIR, '').strip('/'))
                content = common.read(filename)

                if obj in content:
                    li = tokens.ListItem(ul)
                    lang = common.get_language(filename)
                    code = tokens.Code(None, language=lang, code=content)
                    floats.ModalLink(li,
                                     url=unicode(filename),
                                     bottom=True,
                                     content=code,
                                     string=local,
                                     title=tokens.String(None, content=local))

        if ul.children:
            tokens.Heading(parent, string=u'Tests for {}'.format(obj), level=2)
            ul.parent = parent

        return parent
