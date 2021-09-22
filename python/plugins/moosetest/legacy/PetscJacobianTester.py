from moosetools.moosetest.base import make_differ
from ..runners import MOOSEAppRunner
from ..differs import PETScJacobianDiffer

class PetscJacobianTester(MOOSEAppRunner):
    """
    Run MOOSE application with PETSc options for performing Jacobian checks.

    Direct replacement for legacy TestHarness PetscJacobianTester Tester object.
    """

    @staticmethod
    def validParams():
        params = MOOSEAppRunner.validParams()

        # Add parameters from the Differ object
        differ_params = PETScJacobianDiffer.validParams()
        params.append(differ_params, 'ratio_tol', 'difference_tol', 'only_final_jacobian')

        # TODO: Deprecated, see MOOSEAppRunner
        params.add('state', doc="Not used. This parameter is was used for PETSc <3.9, which we no longer support.")
        params.add('run_sim', default=False, doc="Whether to actually run the simulation, testing the Jacobian "
                                                 "at every non-linear iteration of every time step. This is only "
                                                 "relevant for petsc versions >= 3.9.")
        params.add('turn_off_exodus_output', default=True, doc="Whether to set exodus=false in Outputs")
        return params

    def __init__(self, *args, **kwargs):
        MOOSEAppRunner.__init__(self, *args, **kwargs)

        # Apply deprecated parameters
        # TODO: Remove these in favor of 'jacobian' parameter from MOOSEAppRunner and just
        #       use Outputs/exodus=false in 'cli_args' in specification.
        self.parameters().setValue('jacobian', 'TEST_AND_RUN' if self.getParam('run_sim') else 'TEST')
        if self.getParam('turn_off_exodus_output'):
            cli_args = self.getParam('cli_args') or ''
            cli_args += ' Outputs/exodus=false'
            self.parameters().setValue('cli_args', cli_args.strip())

        # Limit to METHOD=OPT
        self.parameters().setValue('libmesh', 'methods', ('opt',))

        # Get parameters from the Runner that should be applied to the Differ
        kwargs = dict()
        kwargs['ratio_tol'] = self.getParam('ratio_tol')
        kwargs['difference_tol'] = self.getParam('difference_tol')
        kwargs['only_final_jacobian'] = self.getParam('only_final_jacobian')

        # Create and add the Differ
        controllers = self.getParam('_controllers')
        jac_differ = make_differ(PETScJacobianDiffer, controllers, name='jacobiandiff', **kwargs)
        self.parameters().setValue('differs', (jac_differ,))
