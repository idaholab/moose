from TestHarness import ValidationCase

class TestCase(ValidationCase):
    def testValidation(self):
        raise Exception('foo')
