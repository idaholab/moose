from TestHarness import ValidationCase

class TestCase(ValidationCase):
    def initialize(self):
        with open('validation_ok.csv', 'r') as f:
            self.value = float(f.read().strip())

    def testValidation(self):
        self.addFloatData('number', self.value, 'K', 'Number',
                          bounds=(self.value - 5.0, self.value + 6.0))
