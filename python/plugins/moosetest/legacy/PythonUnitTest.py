from moosetools import moosetest

from .RunApp import RunApp
from ..controllers import MOOSEConfigController, PETScConfigController, LibMeshConfigController

class PythonUnitTest(moosetest.runners.PythonUnittest):
    @staticmethod
    def validParams():
        params = moosetest.runners.PythonUnittest.validParams()

        # TODO: Remove legacy parameters
        params.add('test_case')
        params.add('required_python_packages', array=True, vtype=str)


        params.append(MOOSEConfigController.validObjectParams(), 'ad_indexing_type')

        params.add('method', vtype=str, array=True, doc="Replaced by 'libmesh_methods'.")
        params.append(LibMeshConfigController.validObjectParams(), 'methods')

        params.append(PETScConfigController.validObjectParams(), 'mumps')

        params.add('prereq', vtype=str, doc="Replaced by 'requires'.")

        # Not actually used, they don't mean anything for running a python unit test; however, the
        # tests using this should be marked as heavy
        params.add('min_parallel', vtype=int, doc="Replaced by 'mpi_min'.")
        params.add('heavy', vtype=bool, doc="Replaced by 'tag_names'.")


        params.add('max_buffer_size') # still used?


        # TODO: Create SQAController
        params.add('design')
        params.add('requirement')
        params.add('issues')
        params.add('detail')
        params.add('deprecated')


        return params

    @staticmethod
    def validCommandLineArguments(parser, params):

        # Legacy Support
        parser.add_argument('--heavy', action='store_true', help="Only execute tests with 'HEAVY' tag.")


    def _setup(self, args):
        moosetest.runners.PythonUnittest._setup(self, args)

        tag = self.getParam('tag')
        if args.heavy:
            tag.setValue('names', ['HEAVY'])


    def __init__(self, *args, **kwargs):
        moosetest.runners.PythonUnittest.__init__(self, *args, **kwargs)

        # TODO: Remove legacy parameters
        self.parameters().setValue('test_cases', (self.getParam('test_case'),))
        self.parameters().setValue('env', 'python_required_packages', self.getParam('required_python_packages') or tuple())
        self.parameters().setValue('moose', 'ad_indexing_type', self.getParam('ad_indexing_type'))
        self.parameters().setValue('libmesh', 'methods', self.getParam('methods') or tuple())
        self.parameters().setValue('petsc', 'mumps', self.getParam('mumps'))
        self.parameters().setValue('requires', self.getParam('prereq'))


        if self.getParam('heavy'):
            self.parameters().setValue('tag_names', ('HEAVY',))
