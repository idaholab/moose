"""
Extension for adding commands to Markdown syntax.
"""
import re

from MooseDocs import common
from MooseDocs.base import components

# Documenting all these classes is far to repetitive and useless.
#pylint: disable=missing-docstring

def make_extension():
    return CommandExtension()

class CommandExtension(components.Extension):

    def init(self, translator):
        components.Extension.init(self, translator)

        # Create a location to store the commands. I have tried this a few different ways, but
        # settle on the following, despite its hackishness. First, the commands were stored on the
        # Reader, which I didn't like because I want this extension to be stand-alone. Second, I
        # stored the commands with a static member on this class, but that fails when multiple
        # instances of the translator are created and this method is called again with other
        # instances active. So, my solution is to just sneak the storage into the current translator
        # object.
        if not hasattr(self.translator, '__EXTENSION_COMMANDS__'):
            setattr(self.translator, '__EXTENSION_COMMANDS__', dict())

    def addCommand(self, command):

        # Type checking
        common.check_type('command', command, CommandComponent)
        common.check_type('COMMAND', command.COMMAND, str)
        common.check_type('SUBCOMMAND', command.SUBCOMMAND, (type(None), str, tuple))

        # Initialize the component
        command.init(self.translator)
        command.extension = self

        # Subcommands can be tuples
        if not isinstance(command.SUBCOMMAND, tuple):
            subcommands = tuple([command.SUBCOMMAND])
        else:
            subcommands = command.SUBCOMMAND

        # Add the command and error if it exists
        for sub in subcommands:
            pair = (command.COMMAND, sub)
            if pair in self.translator.__EXTENSION_COMMANDS__:
                msg = "A CommandComponent object exists with the command '{}' and subcommand '{}'."
                raise common.exceptions.MooseDocsException(msg, pair[0], pair[1])

            self.translator.__EXTENSION_COMMANDS__[pair] = command

    def extend(self, reader, renderer):
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

    def createToken(self, info, parent):

        cmd = (info['command'], info['subcommand'])
        settings = info['settings']

        # Handle filename and None subcommand
        match = self.FILENAME_RE.search(info['subcommand']) if info['subcommand'] else None
        if match:
            cmd = (info['command'], match.group('ext'))
        elif info['subcommand'] and info['subcommand'].startswith('http'):
            cmd = (info['command'], None)
        elif info['subcommand'] and '=' in info['subcommand']:
            #settings += ' ' + info['subcommand']
            settings = info['subcommand'] + ' ' + settings
            cmd = (info['command'], None)

        # Locate the command object to call
        try:
            obj = self.translator.__EXTENSION_COMMANDS__[cmd]
        except KeyError:
            try:
                obj = self.translator.__EXTENSION_COMMANDS__[(cmd[0], '*')]
            except KeyError:
                msg = "The following command combination is unknown: '{} {}'."
                raise common.exceptions.TokenizeException(msg.format(*cmd))

        # Build the token
        if obj.PARSE_SETTINGS:
            settings, _ = common.parse_settings(obj.defaultSettings(), settings)
            obj.setSettings(settings)
        token = obj.createToken(info, parent)
        return token

class BlockInlineCommand(CommandBase):
    RE = re.compile(r'(?:\A|\n{2,})^'           # block begin with empty line
                    r'!(?P<command>\w+)(?: |$)' # command followed by space or end of line
                    r'(?P<subcommand>\S+)?'     # optional subcommand
                    r' *(?P<settings>.*?)'      # optional settings, which can span lines
                    r'(?P<inline>^\S.*?)?'      # content begins when line starts with a character
                    r'(?=\n*\Z|\n{2,})',        # ends with empty line or end of string
                    flags=re.UNICODE|re.MULTILINE|re.DOTALL)

class BlockBlockCommand(CommandBase):
    RE = re.compile(r'(?:\A|\n{2,})^'            # block begin with empty line
                    r'!(?P<command>\w+)!(?: |$)' # command followed by space or end of line
                    r'(?P<subcommand>\S+)?'       # optional subcommand
                    r' *(?P<settings>.*?)'       # optional settings, which can span lines
                    r'(?P<block>^\S.*?)'         # content begins when line starts with a character
                    r'(^!\1-end!)'               # content ends with the "end" command
                    r'(?=\n*\Z|\n{2,})',         # ends with empty line or end of string
                    flags=re.UNICODE|re.MULTILINE|re.DOTALL)

class InlineCommand(CommandBase):
    RE = re.compile(r'!{2}(?P<command>\w+) *(?P<subcommand>\S+)? *(?P<settings>.*?)!{2}',
                    flags=re.UNICODE)
