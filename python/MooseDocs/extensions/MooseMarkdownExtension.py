#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
#pylint: enable=missing-docstring

import markdown

class MooseMarkdownExtension(markdown.Extension):
    """
    A wrapper class to define a static method for extracting the default configure options.

    This is need so that the default options can be displayed via a table using the devel extension
    without getting the configuration settings actually being used.
    """

    @staticmethod
    def defaultConfig():
        """
        Return a dict() containing the default extension configuration options.
        """
        return dict()

    def __init__(self, *args, **kwargs):
        self.config = self.defaultConfig()
        super(MooseMarkdownExtension, self).__init__(*args, **kwargs)

    def extendMarkdown(self, md, md_globals):
        super(MooseMarkdownExtension, self).extendMarkdown(md, md_globals)
