#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
"""
An Extension is comprised of Component objects, the objects are used for tokenizeing markdown
and converting tokens to rendered HTML.
"""
from ..common import mixins

class Extension(mixins.ConfigObject, mixins.TranslatorObject):
    """
    Base class for creating extensions. An extension is simply a mechanism to allow for
    the creation of reader/renderer components to be added to the translation process.

    All aspects of the MooseDocs system rely on Extension objects. These extensions are passed
    to the Translator object. The translator calls the extend method of the extension.

    Inputs:
        kwargs: All key-value pairs are treated as configure options, see ConfigObject.
    """
    __TRANSLATOR_METHODS__ = ['init', 'initPage',
                              'preExecute', 'postExecute',
                              'preRead', 'postRead',
                              'preTokenize', 'postTokenize',
                              'preRender', 'postRender',
                              'preWrite', 'postWrite']

    @staticmethod
    def defaultConfig():
        """Basic Extension configuration options."""
        config = mixins.ConfigObject.defaultConfig()
        config['active'] = (True, "Toggle for disabling the extension. This only changes "
                                  "the initial active state, use setActive to control at runtime.")
        return config

    def __init__(self, **kwargs):
        mixins.ConfigObject.__init__(self, self.__class__.__name__.split('.')[-1].replace('Extension', '').lower(), **kwargs)
        mixins.TranslatorObject.__init__(self)
        self.__requires = set()
        self.__active = self.get('active')

    @property
    def active(self):
        """Return the 'active' status of the Extension."""
        return self.__active

    def setActive(self, value):
        """
        Set the active state for the extension.
        """
        self.__active = value

    def extend(self, reader, renderer):
        """
        Method for adding reader and renderering components.
        """
        pass

    def requires(self, *args):
        """
        Require that the supplied extension module exists within the Translator object. This
        method cannot be called before init().
        """
        self.__requires.update(args)

    def init(self):
        """
        Called after Translator is set, prior to initializing pages.
        """
        pass

    def preExecute(self):
        """
        Called by Translator prior to beginning conversion.
        """
        pass

    def postExecute(self):
        """
        Called by Translator after all conversion is complete.
        """
        pass

    def preRead(self, page):
        """
        Called after to reading the file.

        Input:
             page[pages.Source]: The source object representing the content
        """
        pass

    def postRead(self, page, content):
        """
        Called after to reading the file.

        Input:
             content[str]: The content read from the page
             page[pages.Source]: The source object representing the content
        """
        pass

    def preTokenize(self, page, content, ast):
        """
        Called by Translator prior to tokenization.

        Inputs:
            page[pages.Source]: The source object representing the content
            content[str]: The content read from the page
            ast[tokens.Token]: The root node of the token tree
        """
        pass

    def postTokenize(self, page, ast):
        """
        Called by Translator after tokenization.

        Inputs:
            page[pages.Source]: The source object representing the content
            ast[tokens.Token]: The root node of the token tree
        """
        pass

    def preRender(self, page, ast, result):
        """
        Called by Translator prior to rendering.

        Inputs:
            page[pages.Source]: The source object representing the content
            ast[tokens.Token]: The root node of the token tree
            result[tree.base.NodeBase]: The root node of the result tree
        """
        pass

    def postRender(self, page, result):
        """
        Called by Translator after rendering.

        Inputs:
            page[pages.Source]: The source object representing the content
            result[tree.base.NodeBase]: The root node of the result tree
        """
        pass

    def preWrite(self, page, result):
        """
        Called after renderer has written content.

        Inputs:
            page[pages.Source]: The source object representing the content
            result[tree.base.NodeBase]: The root node of the result tree
        """
        pass

    def postWrite(self, page):
        """
        Called after renderer has written content.

        Inputs:
            page[pages.Source]: The source object representing the content
        """
        pass

    def setAttribute(self, *args):
        """
        Set a global attribute to be communicated across processors.

        This is designed to be called from the <pre/post><Read/Tokenize/Render/Write> methods
        """
        self.translator.executioner.setGlobalAttribute(*args)

    def getAttribute(self, *args):
        """
        Get a global attribute to be communicated across processors.

        This is designed to be called from the <pre/post><Read/Tokenize/Render/Write> methods
        """
        return self.translator.executioner.getGlobalAttribute(*args)

    def getAttributeItems(self):
        """
        Return an iterator to the global attributes to be communicated across processors.

        This is designed to be called from the <pre/post><Read/Tokenize/Render/Write> methods
        """
        return self.translator.executioner.getGlobalAttributeItems()
