from moosetools import moosetest
from .RunApp import RunApp

class XMLDiff(RunApp):
    """
    Run MOOSE application and compare XML files.

    Direct replacement for legacy TestHarness XMLDiff Tester object.
    """
    @staticmethod
    def validParams():
        params = RunApp.validParams()
        params.add('xmldiff', vtype=str, array=True,
                   doc="XML file(s) to compare with counterpart in 'gold' directory.")

        # Add parameters from the Differ object
        differ_params = moosetest.differs.XMLDiffer.validParams()
        params.append(differ_params, 'abs_err', 'rel_err')

        return params

    def __init__(self, *args, **kwargs):
        RunApp.__init__(self, *args, **kwargs)

        # Get parameters from the Runner that should be applied to the Differ
        kwargs = dict()
        kwargs['file_names_created'] = self.getParam('xmldiff')
        kwargs['abs_err'] = self.getParam('abs_err')
        kwargs['rel_err'] = self.getParam('rel_err')

        # Create and add the Differ
        controllers = self.getParam('_controllers')
        xml_differ = moosetest.base.make_differ(moosetest.differs.XMLDiffer, controllers, name='xmldiff', **kwargs)
        self.parameters().setValue('differs', (xml_differ,))
