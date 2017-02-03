import logging
log = logging.getLogger(__name__)

from markdown.util import etree
from MooseSyntaxBase import MooseSyntaxBase
import MooseDocs

class MooseActionList(MooseSyntaxBase):
    """
    Creates dynamic lists for Moose syntax.

    Examples:
    !systems
    !systems framework phase_field
    """

    RE = r'^!systems\s*(.*)'

    def __init__(self, yaml=None, syntax=None, **kwargs):
        MooseSyntaxBase.__init__(self, self.RE, syntax=syntax, **kwargs)
        self._settings['groups'] = None
        self._settings['show_hidden'] = False
        self._yaml = yaml

    def handleMatch(self, match):
        """
        Handle the regex match for this extension.
        """

        # Extract settings
        settings = self.getSettings(match.group(2))

        # Extract the data to consider
        groups = self._syntax.keys()
        if settings['groups']:
            groups = settings['groups'].split()

        # Build complete list of unique action objects
        actions = []
        keys = set()
        for syn in self._syntax.itervalues():
            for value in syn.actions().values():
                if value.key not in keys:
                    actions.append(value)
                    keys.add(value.key)

        # Create the primary element
        el = etree.Element('div')
        el.set('class', 'moose-system-list')

        # Alphabetize actions
        actions.sort(key=lambda action: action.name)

        # Storage structure for <div> tags to allow for nested item creation without
        # the need for complete actions tree or sorted action objects.
        folder_divs = dict()

        # Loop over keys
        for action in actions:

            # Do nothing if the syntax is hidden
            if (action.group in groups) and action.hidden and (not settings['show_hidden']):
                continue

            # Attempt to build the sub-objects table
            collection = MooseDocs.extensions.create_object_collection(action.key, self._syntax, groups=groups, show_hidden=settings['show_hidden'])

            # Do nothing if the table is empty or the supplied group is not desired
            if (not collection) and (action.group not in groups):
                continue

            # Loop through the syntax ("folders") and create the necessary html element
            folder = tuple(action.key.strip('/').split('/'))
            div = el
            for i in range(len(folder)):
                current = '/'.join(folder[0:i+1])

                # If a <div> with the current name exists, use it, otherwise create the <div> and associated heading
                if current in folder_divs:
                    div = folder_divs[current]
                else:
                    div = etree.SubElement(div, 'div')
                    folder_divs[current] = div

                    h = etree.SubElement(div, 'h{}'.format(str(i+2)))
                    h.text = current
                    h_id = current.replace(' ', '_').lower()
                    h.set('id', h_id)

                    if i == 0:
                        div.set('class', 'section scrollspy')
                        div.set('id', h_id)

                    # Add link to action pages
                    a = etree.SubElement(h, 'a')
                    a.set('href', action.markdown)
                    i = etree.SubElement(a, 'i')
                    i.set('class', 'material-icons')
                    i.text = 'input'

                    # Create a chip showing where the action is defined
                    tag = etree.SubElement(h, 'div')
                    tag.set('class', 'chip moose-chip')
                    tag.text = action.group

            if collection:
                div.append(collection)

        return el
