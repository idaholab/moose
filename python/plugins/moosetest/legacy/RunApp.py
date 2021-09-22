from moosetools import moosetest

from ..runners import MOOSEAppRunner
from ..differs import PETScJacobianDiffer

class RunApp(MOOSEAppRunner):
    """
    Run MOOSE application and perform basic output checks.

    Direct replacement for legacy TestHarness RunApp Tester object.
    """
    @staticmethod
    def validParams():
        params = MOOSEAppRunner.validParams()
        params.setValue('allow_deprecated', True)

        # TODO: Deprecate in favor of ConsoleDiff parameters (text_in, re_match, ...)
        params.add('absent_out', vtype=str,
                   doc="Ensure that the supplied text is not found the output text.")
        params.add('expect_out', vtype=str,
                   doc="Ensure that the supplied regex is found in the output text.")
        params.add('match_literal', vtype=bool, default=False,
                   doc="Use direct text comparison rather than regex for 'expect_out'.")

        params.add('errors')

        return params

    def __init__(self, *args, **kwargs):
        MOOSEAppRunner.__init__(self, *args, **kwargs)

        # Get parameters from the Runner that should be applied to the Differ
        kwargs = dict()
        kwargs['re_not_match'] = self.getParam('absent_out')
        if self.getParam('match_literal'):
            kwargs['text_in'] = self.getParam('expect_out')
        else:
            kwargs['re_match'] = self.getParam('expect_out')

        # Create and add the Differ
        controllers = self.getParam('_controllers')
        c_differ = moosetest.base.make_differ(moosetest.differs.ConsoleDiffer, controllers, name='consolediff', **kwargs)
        self.parameters().setValue('differs', (c_differ,))

    def execute(self, *args, **kwargs):
        if self.parameters().isSetByUser('command'):
            return moosetest.runners.ExecuteCommand.execute(self, *args, **kwargs)
        else:
            return MOOSEAppRunner.execute(self, *args, **kwargs)
