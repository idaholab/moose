from TestHarness import ValidationCase
import pandas as pd

class TestCase(ValidationCase):
    def initialize(self):
        csv_file = self.getTesterOutputs(extension='csv')[0]
        df = pd.read_csv(csv_file)
        self.value = float(df['value'].iloc[-1])

    def testValidation(self):
        self.addFloatData('number', self.value, 'coolunits', 'Number',
                          bounds=(self.value - 5.0, self.value + 6.0))
