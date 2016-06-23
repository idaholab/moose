from markdown.preprocessors import Preprocessor
from markdown.util import etree

class MooseSlideTreeprocessor(Preprocessor):
    """
    A slide pre-processor for creating presentations.
    """
    def __init(self, *args):
        pass

    def run(self, root):

        new_root = etree.Element('div')
        section = etree.SubElement(new_root, 'section')

        for child in root:
            if child.text == u'!---':
                section = etree.SubElement(new_root, 'section')
            else:
                new = etree.SubElement(section, child.tag)
                new.append(child)

        return new_root
