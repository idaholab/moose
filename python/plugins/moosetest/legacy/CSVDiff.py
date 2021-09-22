from moosetools.moosetest.base import make_differ
from moosetools.moosetest.differs import CSVDiffer
from .RunApp import RunApp

class CSVDiff(RunApp):
    """
    Run MOOSE application and compare CSV files.

    Direct replacement for legacy TestHarness CSVDiff Tester object.
    """
    @staticmethod
    def validParams():
        params = RunApp.validParams()
        params.add('csvdiff', vtype=str, array=True,
                   doc="CSV file(s) to compare with counterpart in 'gold' directory.")

        # Add parameters from the Differ object
        differ_params = CSVDiffer.validParams()
        params.append(differ_params, 'abs_zero', 'rel_err', 'override_columns', 'override_rel_err', 'override_abs_zero', 'comparison_file')

        return params

    def __init__(self, *args, **kwargs):
        RunApp.__init__(self, *args, **kwargs)

        # Get parameters from the Runner that should be applied to the Differ
        kwargs = dict()
        kwargs['file_names_created'] = self.getParam('csvdiff')
        kwargs['abs_zero'] = self.getParam('abs_zero')
        kwargs['rel_err'] = self.getParam('rel_err')
        kwargs['override_columns'] = self.getParam('override_columns') or tuple()
        kwargs['override_rel_err'] = self.getParam('override_rel_err') or tuple()
        kwargs['override_abs_zero'] = self.getParam('override_abs_zero') or tuple()
        kwargs['comparison_file'] = self.getParam('comparison_file' )

        # Create and add the Differ
        controllers = self.getParam('_controllers')
        csv_differ = make_differ(CSVDiffer, controllers, name='csvdiff', **kwargs)
        self.parameters().setValue('differs', (csv_differ,))
