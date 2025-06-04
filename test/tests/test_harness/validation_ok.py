# Import the base class that will be used for all validation cases
from TestHarness import ValidationCase
# Convenience import for loading app CSV output with pandas
import pandas as pd

# Define the derived test class; the name of this class does not
# matter, as long as it derives from ValidationCase
class TestCase(ValidationCase):
    # Add extra parameters for this validation case; this will enable us
    # to use the same test case for multiple tests for values that differ
    @staticmethod
    def validParams():
        params = ValidationCase.validParams()
        params.addRequiredParam('validation_lower_bound', 'The upper bound for the data')
        params.addRequiredParam('validation_upper_bound', 'The lower bound for the data')
        return params

    # Specific method that is ran once before all "testXXX"
    # methods. Should primarily be used to load any data from
    # the case that was ran
    def initialize(self):
        # Get the CSV file that the CSVDiff tester produces.
        # We could manually search for the file, but instead
        # we use getTesterOutputs() to get the paths to the
        # output files that the TestHarness Tester was reading
        # from (in this case, from the CSVDiff)
        csv_file = self.getTesterOutputs(extension='csv')[0]
        df = pd.read_csv(csv_file)
        # Store the single value from the output that we will
        # use within a test
        self.value = float(df['value'].iloc[-1])

        # Load the lower and upper bounds from the extended parameters
        self.lower_bound = float(self.getParam('validation_lower_bound'))
        self.upper_bound = float(self.getParam('validation_upper_bound'))

    # Define a test case; any defined method that starts with
    # "test" will be ran as a test case
    def testValidation(self):
        # Add float data for validation with the key 'number',
        # the given value, some arbitrary units, a readable
        # description "Number" and our defined bounds
        self.addFloatData('number', self.value, 'coolunits', 'Number',
                          bounds=(self.lower_bound, self.upper_bound))
