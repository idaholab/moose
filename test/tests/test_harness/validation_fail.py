from TestHarness.validation import ValidationCase

class TestCase(ValidationCase):
    def testValidation(self):
        self.addScalarData('number', 100.0, 'Number', None, bounds=(101.0, 200.0))
