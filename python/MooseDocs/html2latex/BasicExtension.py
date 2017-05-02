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

import mooseutils
from Extension import Extension
import elements

class BasicExtension(Extension):
    """
    Basic set of tag Element conversion objects.
    """

    def extend(self, translator):
        """
        Add the basic Element objects to the Translator instance.
        """
        super(BasicExtension, self).extend(translator)

        # Tags that con't need any special handling
        do_nothing = ['html', 'body', 'span', 'div']
        for item in do_nothing:
            translator.elements.add(item, elements.Element(name=item))

        # Headings
        config = self.getConfigs()
        headings = config['headings']
        if headings and len(headings) != 6:
            raise mooseutils.MooseException("Invalid headings list, you must supply a list of 6 "
                                            "valid latex commands.")
        for i, h in enumerate(headings):
            name = 'h{}'.format(i+1)
            translator.elements.add(name, elements.Heading(name=name, command=h))

        translator.elements.add('pre_code', elements.PreCode())
        translator.elements.add('pre', elements.Environment(name='pre', command='verbatim'))
        translator.elements.add('code', elements.ArgumentCommand(name='code', command='texttt'))
        translator.elements.add('ol', elements.Environment(name='ol', command='enumerate'))
        translator.elements.add('ul', elements.Environment(name='ul', command='itemize'))
        translator.elements.add('hr', elements.Command(name='hr',
                                                       command='hrule',
                                                       begin_prefix='\n',
                                                       begin_suffix='\n'))
        translator.elements.add('equation',
                                elements.Environment(name='script',
                                                     command='equation',
                                                     attrs={'type':'math/tex; mode=display'}))
        translator.elements.add('inline_equation', elements.Element(name='script',
                                                                    attrs={'type':'math/tex'},
                                                                    begin='$',
                                                                    end='$'))
        translator.elements.add('table', elements.Table())
        translator.elements.add('thead', elements.TableHeaderFooter(name='thead'))
        translator.elements.add('tfoot', elements.TableHeaderFooter(name='tfoot'))

        translator.elements.add('td', elements.TableItem(name='td'))
        translator.elements.add('th', elements.TableItem(name='th'))
        translator.elements.add('tr', elements.Element(name='tr', close='\n'))
        translator.elements.add('a', elements.LinkElement())
        translator.elements.add('li', elements.ListItem())
        translator.elements.add('p', elements.Command(name='p',
                                                      command='par',
                                                      begin_suffix='\n',
                                                      begin_prefix='\n'))
        translator.elements.add('em', elements.ArgumentCommand(name='em', command='emph'))
        translator.elements.add('figure', elements.Figure())
        translator.elements.add('img', elements.Image())
        translator.elements.add('figcaption', elements.ArgumentCommand(name='figcaption',
                                                                       command='caption'))
        translator.elements.add('center', elements.Environment(name='center', command='center'))
