#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

"""Extension for adding commands to Markdown syntax."""
#pylint: disable=missing-docstring
import re

from MooseDocs import common
from MooseDocs.base import components, Reader
from MooseDocs.tree import tokens
from MooseDocs.extensions import core

def make_extension(**kwargs):
    """Create the CommandExtension object."""
    return CommandExtension(**kwargs)

class CommandExtension(components.Extension):
    """Extension for creating tools necessary generic commands."""
    EXTENSION_COMMANDS = dict()

    def addCommand(self, reader, command):

        # Type checking
        common.check_type('reader', reader, Reader)
        common.check_type('command', command, CommandComponent)
        common.check_type('COMMAND', command.COMMAND, str)
        common.check_type('SUBCOMMAND', command.SUBCOMMAND, (type(None), str, tuple))

        # Initialize the component
        command.setReader(reader)
        command.setExtension(self)

        # Subcommands can be tuples
        if not isinstance(command.SUBCOMMAND, tuple):
            subcommands = tuple([command.SUBCOMMAND])
        else:
            subcommands = command.SUBCOMMAND

        # Add the command and error if it exists
        for sub in subcommands:
            pair = (command.COMMAND, sub)
            if pair in CommandExtension.EXTENSION_COMMANDS:
                msg = "A CommandComponent object exists with the command '{}' and subcommand '{}'."
                raise common.exceptions.MooseDocsException(msg, pair[0], pair[1])

            CommandExtension.EXTENSION_COMMANDS[pair] = command

    def extend(self, reader, renderer):
        self.requires(core)
        reader.addBlock(BlockBlockCommand(), location='_begin')
        reader.addBlock(BlockInlineCommand(), location='<BlockBlockCommand')
        reader.addInline(InlineCommand(), location='_begin')

class CommandComponent(components.TokenComponent): #pylint: disable=abstract-method
    COMMAND = None
    SUBCOMMAND = None

class CommandBase(components.TokenComponent):
    """
    Provides a component for creating commands.

    A command is defined by an exclamation mark followed by a keyword and optionally a sub-command.

    This allows all similar patterns to be handled by a single regex, which should aid in parsing
    speed as well as reduce the burden of adding new commands.

    New commands are added by creating a CommandComponent object and adding this component to the
    MarkdownExtension via the addCommand method (see extensions/devel.py for an example).
    """
    PARSE_SETTINGS = False
    FILENAME_RE = re.compile(r'(?P<filename>\S*\.(?P<ext>\w+))(?= |$)', flags=re.UNICODE)

    def __init__(self, *args, **kwargs):
        components.TokenComponent.__init__(self, *args, **kwargs)

    def createToken(self, parent, info, page):

        cmd = (info['command'], info['subcommand'])
        settings = info['settings']

        # Handle filename and None subcommand
        match = self.FILENAME_RE.search(info['subcommand']) if info['subcommand'] else None
        if match:
            cmd = (info['command'], match.group('ext'))
        elif info['subcommand'] and info['subcommand'].startswith('http'):
            cmd = (info['command'], None)
        elif info['subcommand'] and '=' in info['subcommand']:
            settings = info['subcommand'] + ' ' + settings
            cmd = (info['command'], None)

        # Locate the command object to call
        try:
            obj = CommandExtension.EXTENSION_COMMANDS[cmd]
        except KeyError:
            try:
                obj = CommandExtension.EXTENSION_COMMANDS[(cmd[0], '*')]
            except KeyError:
                msg = "The following command combination is unknown: '{} {}'."
                raise common.exceptions.MooseDocsException(msg.format(*cmd))

        if not obj.extension.active:
            if isinstance(self, InlineCommand):
                tokens.DisabledToken(parent, tag='span', string=info[0])
            tokens.DisabledToken(parent, tag='p', string=info[0])
            return parent

        # Build the token
        if obj.PARSE_SETTINGS:
            settings, _ = common.parse_settings(obj.defaultSettings(), settings)
            obj.setSettings(settings)
        token = obj.createToken(parent, info, page)
        return token

    def setTranslator(self, translator):
        for comp in CommandExtension.EXTENSION_COMMANDS.values():
            comp.setTranslator(translator)

class BlockInlineCommand(CommandBase):
    RE = re.compile(r'(?:\A|\n{2,})^'           # block begin with empty line
                    r'!(?P<command>\w+)(?: |$)' # command followed by space or end of line
                    r'(?P<subcommand>\S+)?'     # optional subcommand
                    r' *(?P<settings>.*?)'      # optional settings, which can span lines
                    r'(?P<block>^\S.*?)?'       # content begins when line starts with a character
                    r'(?=\n*\Z|\n{2,})',        # ends with empty line or end of string
                    flags=re.UNICODE|re.MULTILINE|re.DOTALL)

class BlockBlockCommand(CommandBase):
    RE = re.compile(r'(?:\A|\n{2,})^'            # block begin with empty line
                    r'!(?P<command>\w+)!(?: |$)' # command followed by space or end of line
                    r'(?P<subcommand>\S+)?'      # optional subcommand
                    r' *(?P<settings>.*?)'       # optional settings, which can span lines
                    r'(?P<block>^\S.*?)'         # content begins when line starts with a character
                    r'(^!\1-end!)'               # content ends with the "end" command
                    r'(?=\n*\Z|\n{2,})',         # ends with empty line or end of string
                    flags=re.UNICODE|re.MULTILINE|re.DOTALL)

class InlineCommand(CommandBase):
    RE = re.compile(r'!{2}(?P<command>\w+) *(?P<subcommand>\w+)? *(?P<settings>.*?)!{2}',
                    flags=re.UNICODE)
