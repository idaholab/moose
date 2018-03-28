#pylint: disable=missing-docstring
from MooseDocs.extensions import command
from MooseDocs.tree import tokens

def make_extension(**kwargs):
    return PanopticExtension(**kwargs)

class PanopticExtension(command.CommandExtension):
    """
    Allows global shortcuts to be defined within the configure file.

    panoptic means "being or presenting a comprehensive", which is a fancy name for global. "global"
    is a python keyword, so I don't want to use it.
    https://www.merriam-webster.com/dictionary/panoptic
    """

    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        config['shortcuts'] = (dict(), "Key-value pairs to insert as shortcuts, this should be " \
                                       "a dictionary or a dictionary of dictionaries.")
        return config

    def extend(self, reader, renderer):
        pass

    def postTokenize(self, ast, config): #pylint: disable=unused-argument
        shortcuts = self.get('shortcuts', dict())
        for key, value in shortcuts.iteritems():
            if isinstance(value, dict):
                for k, v in value.iteritems():
                    tokens.Shortcut(ast, key=unicode(k), link=unicode(v), string=unicode(k))
            else:
                tokens.Shortcut(ast, key=unicode(key), link=unicode(value), string=unicode(key))
