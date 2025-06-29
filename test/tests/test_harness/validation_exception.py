from TestHarness.validation import ValidationCase

class TestCase(ValidationCase):
    def testValidation(self):
        raise Exception('foo')
