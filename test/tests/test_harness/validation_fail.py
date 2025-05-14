from TestHarness import ValidationCase

class TestCase(ValidationCase):
    def testValidation(self):
        self.addFloatData('number', 100.0, None, 'Number',
                          bounds=(101.0, 200.0))
