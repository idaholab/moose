from markdown.treeprocessors import InlineProcessor
import logging
log = logging.getLogger(__name__)

from MooseImageFile import MooseImageFile

class MooseFigure(MooseImageFile):
    """
    Defines syntax for adding images as numbered figures with labels that can be referenced (see MooseFigureReference)
    """
    RE = r'^!figure\s+(.*?)(?:$|\s+)(.*)'

    def __init__(self, *args, **kwargs):
        super(MooseFigure, self).__init__(*args, **kwargs)

        self._settings['prefix'] = 'Figure'
        self._settings['id'] = None
        self._figure_number = 0

    def initialize(self):
        """
        Reset the figure numbering prior to processing the markdown for this page.
        """
        self._figure_number = 0

    def handleMatch(self, match):
        """
        Creates the html for a numbered MOOSE figure.
        """

        # Increment the image number
        self._figure_number += 1

        # Extract information from the regex match
        rel_filename = match.group(2)
        settings = self.getSettings(match.group(3))

        # Error if the 'label' setting is not provided
        if not settings['id']:
            return self.createErrorElement("The 'id' setting must be supplied for the figure: {}".format(rel_filename))
        else:
            settings['id'] = settings['id'].replace(':', '')

        # Update the caption to include the numbered prefix
        if settings['caption']:
            settings['caption'] = '{} {}: {}'.format(settings['prefix'], self._figure_number, settings['caption'])
        else:
            settings['caption'] = '{} {}'.format(settings['prefix'], self._figure_number)

        # Create the element and store the number
        el = self.createImageElement(rel_filename, settings)
        el.set('data-moose-figure-number', str(self._figure_number))
        return el
