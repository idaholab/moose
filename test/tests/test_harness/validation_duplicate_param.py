from TestHarness.validation import ValidationCase

class TestCase(ValidationCase):
    @staticmethod
    def validParams():
        params = ValidationCase.validParams()
        params.addRequiredParam('type', 'Docs')
        return params
