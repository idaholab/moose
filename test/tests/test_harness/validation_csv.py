from TestHarness.validation import CSVValidationCase

class TestCase(CSVValidationCase):
    def test(self):
        self.addScalarCSVData('value', 0, 'Element average value, index 0', None, store_key='value_0')
        self.addScalarCSVData('value', 1, 'Element average value, index 1', None, store_key='value_1')
        self.addScalarCSVData('value', -1, 'Element average value, index -1', None, store_key='value_last')
