#pylint: disable=missing-docstring,no-member
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
#pylint: enable=missing-docstring
import os
import uuid
import logging
import mooseutils

LOG = logging.getLogger(__name__)

@mooseutils.addProperty('base', ptype=str)                  # set by Translator::init
@mooseutils.addProperty('source', ptype=str, required=True) # supplied source file/directory
class Page(mooseutils.AutoPropertyMixin):
    """
    Base class for input content that defines the methods called by the translator.

    This classes uses properties to minimize modifications after construction.
    """
    def __init__(self, fullname, **kwargs):
        super(Page, self).__init__(**kwargs)
        self._fullname = fullname            # local path of the node
        self._name = fullname.split('/')[-1] # file/folder name
        self.__unique_id = uuid.uuid4()      # a unique identifier

    @property
    def uid(self):
        """Return the unique ID for this page."""
        return self.__unique_id

    @property
    def name(self):
        """Return the name of the page (i.e., the directory or filename)."""
        return self._name

    @property
    def local(self):
        """Returns the local directory/filename."""
        return self._fullname

    @property
    def destination(self):
        """Returns the translator destination location."""
        return os.path.join(self.base, self.local)

    @property
    def depth(self):
        """Returns the local folder depth"""
        return self.local.strip(os.sep).count(os.sep)

    def relativeSource(self, other):
        """Location of this page related to the other page."""
        return os.path.relpath(self.local, os.path.dirname(other.local))

    def relativeDestination(self, other):
        """
        Location of this page related to the other page.

        Inputs:
            other[LocationNodeBase]: The page that this page is relative too.
        """
        return os.path.relpath(self.destination, os.path.dirname(other.destination))

    def __str__(self):
        """Define the anytree screen output."""
        return '{}: {}, {}'.format(mooseutils.colorText(self.__class__.__name__, self.COLOR),
                                   self.local, self.source)

@mooseutils.addProperty('content', ptype=str, default=u'')
class Text(Page):
    """Text only Page node for unit testing."""
    COLOR = 'GREEN'
    def __init__(self, **kwargs):
        super(Text, self).__init__('_text_', source='_text_', **kwargs)

class Directory(Page):
    """
    Directory nodes.
    """
    COLOR = 'CYAN'

class File(Page):
    """
    File nodes.

    General files that need to be copied to the output directory.
    """
    COLOR = 'MAGENTA'

@mooseutils.addProperty('output_extension', ptype=str) # set by Translator::init
class Source(File):
    """
    Node for content that is being converted (e.g., Markdown files).
    """
    COLOR = 'YELLOW'
    def __init__(self, *args, **kwargs):
        super(Source, self).__init__(*args, **kwargs)

    @property
    def destination(self):
        """The content destination (override)."""
        _, ext = os.path.splitext(self.source)
        return super(Source, self).destination.replace(ext, self.output_extension)
