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

from ..common import exceptions, parse_settings, mixins
from ..tree import tokens

class Component(mixins.TranslatorObject):
    """
    Each extension is made up of components, both for tokenizing and rendering. The components
    provide a means for defining settings as well as other customizable features required for
    translation.
    """
    def __init__(self):
        mixins.TranslatorObject.__init__(self)
        self.__extension = None

    @property
    def extension(self):
        """Access to the parent Extension object for this component."""
        return self.__extension

    def setExtension(self, extension):
        """
        Attach the extension the component, this is done by the Translator.
        """
        self.__extension = extension

class ReaderComponent(Component, mixins.ReaderObject):
    """
    Base class for creating components designed to create a token during tokenization.

    # Overview
    ReaderComponent objects are designed to be created by Extension objects and added to the Reader
    object in the Extension:extend method. The purpose of a ReaderComponent is to define a regular
    expression, that when match returns a token that is added to the AST.

    The codebase of MooseDocs (Translator, Reader, Renderer) is certainly capable of tokenizing
    arbitrary formats. However, the primary use case is for creating HTML so there are a certain
    features for this class that are setup with that application in mind.

    ## RE Class Member
    The RE class member variable is used for convenience to
    allow for derived classes to avoid creating an __init__ method. As shown in core.py the purpose
    of this class is to be able to add parser syntax for tokenizing with a minimal amount code, the
    following static member variables aid in the design goal.

    RE must be defined as a compile re expressions that captures the content to be converted to a
    token within the createToken method.  - TOKEN: The token type (not instance) that will be
    created

    ## Settings
    If the supplied regex match option has a group named "settings" these settings will be
    automatically parsed by the translator and used to update the values returned by the
    defaultSettings method. These settings must be key-value pairings that use an equal sign that is
    does not contains spaces on either side (e.g., foo=bar).

    The automatic parsing settings may be disable by setting the PARSE_SETTINGS class member
    variable to False in the child object.
    """
    RE = None
    PARSE_SETTINGS = True

    @staticmethod
    def defaultSettings():
        """
        Default settings for the component. Child classes should define a similar method to create
        the default settings for the component, see core.py for examples.
        """
        settings = dict()
        settings['style'] = (None, "The style settings that are passed to rendered HTML tag.")
        settings['class'] = (None, "The class settings to be passed to rendered HTML tag.")
        settings['id'] = (None, "The class settings to be passed to the rendered tag.")
        return settings

    def __init__(self):
        """
        Constructs the object and sets the default settings of the object.
        """
        Component.__init__(self)
        mixins.ReaderObject.__init__(self)

        # Check return type of default settings
        defaults = self.defaultSettings()
        if not isinstance(defaults, dict):
            msg = "The component '{}' must return a dict from the defaultSettings static method."
            raise exceptions.MooseDocsException(msg, self)

    def __call__(self, parent, info, page):
        """
        MooseDocs internal method, this should not be called, please use the createToken method.

        The lexer system within MooseDocs expects a function this method allows this class to act
        as a function.

        Inputs:
            info[LexerInformation]: Object containing the lexer information object.
            parent[tokens.Token]: The parent node in the AST for the token being created.
        """
        # Disable inactive extensions, the "is not None" allows this method to work when
        # components are added outside of an extension, which is needed for testing
        if (self.extension is not None) and (not self.extension.active):
            tokens.DisabledToken(parent, string=info[0])
            return parent

        # Define the settings
        defaults = self.defaultSettings()
        if self.PARSE_SETTINGS and ('settings' in info):
            settings, _ = parse_settings(defaults, info['settings'])
        else:
            settings = {k:v[0] for k, v in defaults.items()}

        return self.createToken(parent, info, page, settings)

    @staticmethod
    def attributes(settings):
        """
        Return a dictionary with the common html settings.

        This property is only available from within the createToken method, it returns None when
        called externally.
        """
        out = dict()
        if settings.get('style', None) is not None:
            out['style'] = settings['style'].strip()
        if settings.get('id', None) is not None:
            out['id'] = settings['id'].strip()
        if settings.get('class', None) is not None:
            out['class'] = settings['class'].strip()
        return out

    def createToken(self, parent, info, page, settings):
        """
        Method designed to be implemented by child classes, this method should create the
        token for the AST based on the regex match.

        Inputs:
            parent[tokens.Token]: The parent node in the AST for the token being created.
            info[LexerInformation]: Object containing the lexer information object.
            page[page.Page]: Page object of the current content being read.
        """
        raise NotImplementedError("The createToken method is required.")

class RenderComponent(Component, mixins.RendererObject):
    """
    RenderComponent objects are used to convert tokens to an output format such as HTML or LaTeX.

    The function to be called is assigned by the Renderer object; however, it has the following
    signature:
        functionName(self, parent, token, page):
            ...

    This allows the RenderComponent object to have multiple methods for converting to different
    formats. For example, the components in core.py have createHTML and createLatex methods to
    work with the HTMLRenderer and the LatexRenderer.
    """
    def __init__(self):
        """
        Constructs the object and sets the default settings of the object.
        """
        Component.__init__(self)
        mixins.RendererObject.__init__(self)
