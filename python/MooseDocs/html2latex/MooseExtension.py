#pylint: disable=missing-docstring
####################################################################################################
#                                    DO NOT MODIFY THIS HEADER                                     #
#                   MOOSE - Multiphysics Object Oriented Simulation Environment                    #
#                                                                                                  #
#                              (c) 2010 Battelle Energy Alliance, LLC                              #
#                                       ALL RIGHTS RESERVED                                        #
#                                                                                                  #
#                            Prepared by Battelle Energy Alliance, LLC                             #
#                               Under Contract No. DE-AC07-05ID14517                               #
#                               With the U. S. Department of Energy                                #
#                                                                                                  #
#                               See COPYRIGHT for full restrictions                                #
####################################################################################################
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
