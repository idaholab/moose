from moosetools import moosetest

class RunCommand(moosetest.runners.ExecuteCommand):
    """
    Direct replacement for legacy TestHarness RunCommand Tester object.
    """
    @staticmethod
    def validParams():
        params = moosetest.runners.ExecuteCommand.validParams()

        params.add('required_python_packages', vtype=str, array=True, doc="Replaced by 'env_python_required_packages'.")


        # TODO:
        params.add('design')
        params.add('requirement')
        params.add('issues')
        params.add('detail')
        params.add('deprecated')
        params.add('collections')

        return params

    def __init__(self, *args, **kwargs):
        moosetest.runners.ExecuteCommand.__init__(self, *args, **kwargs)
        if self.isParamValid('required_python_packages'):
            self.parameters().setValue('env', 'python_required_packages', self.getParam('required_python_packages'))
