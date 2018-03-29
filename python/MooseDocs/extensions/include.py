#pylint: disable=missing-docstring
import re
from MooseDocs import common
from MooseDocs.common import exceptions
from MooseDocs.extensions import command

assert re

def make_extension(**kwargs):
    return IncludeExtension(**kwargs)

class IncludeExtension(command.CommandExtension):
    """Enables the !include command for including files in other files."""

    def extend(self, reader, renderer):
        self.requires(command)
        self.addCommand(IncludeCommand())

class IncludeCommand(command.CommandComponent):
    COMMAND = 'include'
    SUBCOMMAND = 'md' #TODO: get this from the reader inside the __init__ method.

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['re'] = (None, "Extract content via a regex, if the 'content' group exists it " \
                                 "is used as the desired content, otherwise group zero is used.")
        settings['re-flags'] = ('re.M|re.S|re.U', "Regular expression flags commands pass to the " \
                                                  "Python re module.")
        settings['start'] = (None, "String contained on the starting line.")
        settings['end'] = (None, "String contained on the ending line.")
        return settings

    def createToken(self, info, parent):
        """
        NOTICE:
        Ideally, this method would create a connection between the two pages so that when you
        update the included page the livereload would run the including page. This doesn't work
        because of the multiprocessing, which would be making a connection between object that
        are on copies working on other processes from those that the livereload is watching.

        TODO: A possible fix would be to just hack in a regex for !include into the livereload
              watcher object itself, just need to implement it.
        """

        master_page = self.translator.current
        include_page = master_page.findall(info['subcommand'], exc=exceptions.TokenizeException)[0]

        content = common.read(include_page.source) #TODO: copy existing tokens when not using re
        if self.settings['re']:
            content = common.regex(self.settings['re'], content, eval(self.settings['re-flags']))

        elif self.settings['start'] or self.settings['end']:
            lines = content.split('\n')
            start_idx = None
            end_idx = None
            start = self.settings['start']
            end = self.settings['end']

            for i, line in enumerate(lines):
                if (not start_idx) and (start in line):
                    start_idx = i
                if (not end_idx) and (end in line):
                    end_idx = i

            if start_idx is None:
                start_idx = 0
            if end_idx is None:
                end_idx = -1

            content = lines[start_idx:end_idx]

        self.translator.reader.parse(parent, content)
        return parent
