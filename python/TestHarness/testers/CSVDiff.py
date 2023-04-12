#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from SchemaDiff import SchemaDiff
from TestHarness import util
import os
import csv

class CSVDiff(SchemaDiff):

    @staticmethod
    def validParams():
        params = SchemaDiff.validParams()
        params.addRequiredParam('csvdiff',   [], "A list of files to run CSVDiff on.")
        params.addParam('override_columns',   [], "A list of variable names to customize the CSVDiff tolerances.")
        params.addParam('override_rel_err',   [], "A list of customized relative error tolerances.")
        params.addParam('override_abs_zero',   [], "A list of customized absolute zero tolerances.")
        params.addParam('ignored_columns',      [], "A list of columns to ignore while diffing.")
        return params

    def __init__(self, name, params):
        params['schemadiff'] = params['csvdiff']
        if params['ignored_items']:
            from mooseutils import mooseWarning
            mooseWarning('"ignored_items" aliases to ignored_columns in CSVDiff. Any values given will block diffing on column names that match.')
            params['ignored_columns'] = params['ignored_items']
            params['ignored_items'] = []
        SchemaDiff.__init__(self, name, params)

    def load_file(self, path1):
        try:
            output_dict = {}
            with open(path1, newline='') as csvfile:
                reader = csv.DictReader(csvfile)
                for row in reader:
                # Loop through each column in the row
                    for column, value in row.items():
                        # Simply don't include columns in the ignored columns list in the diff.
                        if column not in self.specs['ignored_columns']:
                            # If the column is not already in the output dictionary, add it
                            if column not in output_dict:
                                output_dict[column] = []

                            # Add the value to the list for the corresponding column
                            output_dict[column].append(value)
            return output_dict
        except Exception as e:
            return e

    def do_deepdiff(self,orig, comp, rel_err, abs_zero, exclude_values:list=None):
        diff = ""
        for col in orig:
            if col in self.specs['override_columns']:
                idx = self.specs['override_columns'].index(col)
                if self.specs['override_rel_err']:
                    rel_err = float(self.specs['override_rel_err'][idx])
                if self.specs['override_abs_zero']:
                    abs_zero = float(self.specs['override_abs_zero'][idx])
            col_diff = super().do_deepdiff(orig[col], comp[col], rel_err, abs_zero, exclude_values)
            if col_diff:
                diff += col_diff
        return diff
