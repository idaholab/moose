from moosetools import moosetest
from .RunApp import RunApp

class JSONDiff(RunApp):
    """
    Run MOOSE application and compare JSON files.

    Direct replacement for legacy TestHarness JSONDiff Tester object.
    """
    @staticmethod
    def validParams():
        params = RunApp.validParams()
        params.add('jsondiff', vtype=str, array=True,
                   doc="JSON file(s) to compare with counterpart in 'gold' directory.")

        # Add parameters from the Differ object
        differ_params = moosetest.differs.JSONDiffer.validParams()
        params.append(differ_params, 'abs_err', 'rel_err', 'skip_keys')

        return params

    def __init__(self, *args, **kwargs):
        RunApp.__init__(self, *args, **kwargs)

        # Get parameters from the Runner that should be applied to the Differ
        kwargs = dict()
        kwargs['file_names_created'] = self.getParam('jsondiff')
        kwargs['abs_err'] = self.getParam('abs_err')
        kwargs['rel_err'] = self.getParam('rel_err')
        kwargs['skip_keys'] = self.getParam('skip_keys') or tuple()

        # Create and add the Differ
        controllers = self.getParam('_controllers')
        json_differ = moosetest.base.make_differ(moosetest.differs.JSONDiffer, controllers, name='jsondiff', **kwargs)
        self.parameters().setValue('differs', (json_differ,))
