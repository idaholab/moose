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

from Extension import Extension
import elements
import moose_elements

class MooseExtension(Extension):
    """
    Aggregates the MOOSE specific element objects into an extension for html to latex conversion.
    """
    def __init__(self, **kwargs):
        super(MooseExtension, self).__init__(**kwargs)
        self._configs.setdefault('hrule', False)

    def extend(self, translator):
        config = self.getConfigs()

        translator.elements.add('moose_cite', moose_elements.MooseCite(), '<span')
        translator.elements.add('admonition', moose_elements.Admonition(), '<div')
        translator.elements.add('moose_code_div', moose_elements.MooseCodeDiv(), '_begin')
        translator.elements.add('moose_pre_code', moose_elements.MoosePreCode(), '<pre_code')
        translator.elements.add('moose_table', moose_elements.MooseTable(), '<table')
        translator.elements.add('moose_figure', moose_elements.MooseFigure(), '<div')
        translator.elements.add('moose_img', moose_elements.MooseImage(), '<img')
        translator.elements.add('moose_img_caption', elements.ArgumentCommand(name='p', \
                                command='caption', end_suffix='\n', \
                                attrs={'class':'moose-image-caption'}, strip=True), '<p')
        translator.elements.add('moose_button',
                                elements.Element(name='button', content=''),
                                '_begin')

        if not config['hrule']:
            translator.elements.add('moose_hide_hr', elements.Element(name='hr'), '<hr')
