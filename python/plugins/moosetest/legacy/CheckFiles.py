from moosetools.moosetest.base import make_differ
from moosetools.moosetest.differs import TextFileContentDiffer
from .RunApp import RunApp

class CheckFiles(RunApp):
    """
    Run MOOSE application and compare Exodus files.

    Direct replacement for legacy TestHarness Exodiff Tester object.
    """
    @staticmethod
    def validParams():
        params = RunApp.validParams()
        params.add('check_files', vtype=str, array=True, doc="File(s) that are required to exist.")
        params.add('check_not_exists', vtype=str, doc="File(s) that are required not to exist.")

        # TODO: Deprecate and create 'text_in' and 're_match' for file content
        f_params = params.getValue('file')
        f_params.addParam('expect_out', "A regular expression that must occur in all of the check files in order for the test to be considered passing.")

        params.add('delete_output_folders') # not used


        return params

    def __init__(self, *args, **kwargs):
        RunApp.__init__(self, *args, **kwargs)

        # Get parameters from the Runner that should be applied to the Differ
        kwargs = dict()
        kwargs['file_names_created'] = self.getParam('check_files') or tuple()
        if kwargs['file_names_created']:
            self.parameters().setValue('file', 'check_created', True)
        kwargs['re_match'] = self.getParam('file', 'expect_out')

        # Create and add the Differ
        controllers = self.getParam('_controllers')
        txt_differ = make_differ(TextFileContentDiffer, controllers, name='textdiff', **kwargs)

        differs = list(self.getParam('differs'))
        self.parameters().setValue('differs', tuple([txt_differ, *differs]))
