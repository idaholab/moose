from TestHarness import ValidationCase

class TestCase(ValidationCase):
    def testValidation(self):
        value = 1.0
        self.addFloatData('number', value, None, 'Number',
                          bounds=(value - 0.1, value + 0.1))
