#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import re
import os
import csv
from SchemaDiff import SchemaDiff
from FileTester import FileTester # checkRunnable

class CSVDiff(SchemaDiff):
    @staticmethod
    def validParams():
        params = SchemaDiff.validParams()
        params.addRequiredParam('csvdiff',    [], 'A list of files to run CSVDiff on.')
        params.addParam('override_columns',   [], 'A list of variable names to customize the '
                                                  'CSVDiff tolerances.')
        params.addParam('override_rel_err',   [], 'A list of customized relative error tolerances.')
        params.addParam('override_abs_zero',  [], 'A list of customized absolute zero tolerances.')
        params.addParam('ignore_columns',     [], 'A list of columns names which will not be '
                                                  'included in the comparison.')
        params.addParam('custom_columns',     [], 'A list of select columns which will be '
                                                  'included in the comparison.')
        params.addParam('comparison_file',        'Use supplied custom comparison config file.')
        return params

    @staticmethod
    def augment_csv(csv_dict, ignore_columns:list=None, custom_columns:list=None):
        """ Add or Drop entire columns """
        if ignore_columns is None:
            ignore_columns = []
        if custom_columns is None:
            custom_columns = []

        # Drop everything except what is listed in custom_columns
        if custom_columns:
            _tmp_dict = csv_dict.copy()
            for a_column in _tmp_dict.keys():
                if a_column.lower() not in custom_columns:
                    csv_dict.pop(a_column.lower())

        # Drop what is listed in ignore_columns
        for drop_field in ignore_columns:
            if drop_field.lower() in csv_dict.keys():
                csv_dict.pop(drop_field.lower())
        return csv_dict

    def __init__(self, name, params):
        params['schemadiff'] = params['csvdiff']
        self.custom_params = {}
        SchemaDiff.__init__(self, name, params)

    def getOutputFiles(self):
        return self.specs['csvdiff']

    # Check that override parameter lists are the same length
    def checkRunnable(self, options):
        if ((len(self.specs['override_columns']) != len(self.specs['override_rel_err']))
        or (len(self.specs['override_columns']) != len(self.specs['override_abs_zero']))
        or (len(self.specs['override_rel_err']) != len(self.specs['override_abs_zero']))):
            self.setStatus(self.fail, 'Override inputs not the same length')
            return False

        if (any(x in self.specs['override_columns'] for x in self.specs['ignore_columns'])
        or any(x in self.specs['ignore_columns'] for x in self.specs['override_columns'])
        or any(x in self.specs['custom_columns'] for x in self.specs['ignore_columns'])
        or any(x in self.specs['ignore_columns'] for x in self.specs['custom_columns'])):
            self.setStatus(self.fail, ('Ignored columns cannot also be included in lists of'
                                       ' columns on which csv comparisons will occur.'))
            return False

        return FileTester.checkRunnable(self, options)

    def load_file(self, path):
        """ Load CSV File and return a populated dictionary """
        output_dict = {}
        with open(path, 'r', encoding='utf-8') as csvfile:
            reader = csv.DictReader(csvfile, skipinitialspace=True)
            for row in reader:
                for column, value in row.items():
                    output_dict[column.lower()] = output_dict.get(column.lower(), [])
                    output_dict[column.lower()].append(value.lower())

        # Allow modifications to CSV file
        if self.specs.isValid('ignore_columns') or self.specs.isValid('custom_columns'):
            output_dict = self.augment_csv(output_dict,
                                            self.specs['ignore_columns'],
                                            self.specs['custom_columns'])

        # Allow further modifications based on comparison file
        if self.custom_params:
            output_dict = self.augment_csv(output_dict,
                                            None,
                                            self.custom_params['FIELDS'].keys())

        return output_dict

    def processResults(self, moose_dir, options, output):
        if self.specs.isValid('comparison_file'):
            comparison_file = os.path.join(self.getTestDir(), self.specs['comparison_file'])
            if not os.path.exists(comparison_file):
                self.setStatus(self.fail, 'MISSING COMPARISON FILE')
                return output
            else:
                self.custom_params = self.parseComparisonFile(comparison_file)
        return SchemaDiff.processResults(self, moose_dir, options, output)

    def do_deepdiff(self, orig, comp, rel_err, abs_zero, exclude_values:list=None):
        """ Perform DIFF Comparison and return (diff, error) tuple results """
        (diff, error) = '', None

        # Sanity check on column size. Test Spec options like custom_columns, ignore_columns
        # have already been accounted for (see augment_csv method)
        for column in orig:
            if len(orig[column]) != len(comp[column]):
                diff = f'Columns with header \'{column}\' are not the same length'
                return (diff, error)

        # Override global params if comparison file is used
        if self.custom_params:
            rel_err = abs(float(self.custom_params.get('RELATIVE', self.specs['rel_err'])))
            abs_zero = abs(float(self.custom_params.get('ZERO', self.specs['abs_zero'])))

        for index, column in enumerate(orig):
            _rel_err = abs(rel_err)
            _abs_zero = abs(abs_zero)
            # Apply field params (overrides global for this single field)
            if (column in self.specs['override_columns']
                or (self.custom_params and column in self.custom_params['FIELDS'].keys())):

                # Test-Specification file override
                if column in self.specs['override_columns']:
                    idx = self.specs['override_columns'].index(column)
                    if self.specs['rel_err']:
                        _rel_err = abs(float(self.specs['override_rel_err'][idx]))
                    if self.specs['override_abs_zero']:
                        _abs_zero = abs(float(self.specs['override_abs_zero'][idx]))

                if self.custom_params and column in self.custom_params['FIELDS'].keys():
                    _meta = self.custom_params['FIELDS'][column]
                    _rel_err = abs(max(_rel_err, float(_meta.get('RELATIVE', _rel_err))))
                    _abs_zero = abs(max(_abs_zero, float(_meta.get('ZERO', _abs_zero))))

            for value_id, value in enumerate(orig[column]):
                if value == comp[column][value_id]:
                    continue
                (diff, error) = super().do_deepdiff(value,
                                                    comp[column][value_id],
                                                    _rel_err,
                                                    _abs_zero,
                                                    exclude_values)
                # Overrite global since we are about to exit, with values we diffed with
                if diff:
                    rel_err = _rel_err
                    abs_zero = _abs_zero
                    break
            # neat: https://www.geeksforgeeks.org/how-to-break-out-of-multiple-loops-in-python/
            else:
                continue
            break
        # overwrite diff with formatted information
        if diff:
            diff = (f'Relative Error Tolorance: {rel_err}\n'
                    f'Absolute Zero:            {abs_zero}\n'
                    f'Column {index}: {diff}\n')
        return (diff, error)

    def getParamValues(self, param, param_line):
        """ return a list of discovered values for param """
        return re.findall(param + '\s+([0-9e.\-\+]+)', param_line)

    def parseComparisonFile(self, config_file):
        """ Walk through comparison file and populate/return a dictionary as best we can """
        # A set of known parameter naming conventions. The comparison file can have these set, and
        # we will use them. The format of the comparison file is similar to that supplied by
        # exodiff output.
        zero_params = set(['floor', 'abs_zero', 'absolute'])
        tolerance_params = set(['relative', 'rel_tol'])
        custom_params = {'RELATIVE': 0.0, 'ZERO': 0.0, 'FIELDS': {}}

        with open(config_file, 'r') as comparison_file:
            lines = comparison_file.readlines()
            for a_line in lines:
                s_line = a_line.strip()
                words = set(a_line.split())

                # Ignore this line if commented, is the 'time steps' header, or contains logical nots
                if s_line and \
                (s_line.startswith("#")
                    or s_line.lower().find('time steps') == 0
                    or s_line[0] == "!"):
                    continue

                field_key = re.findall(r'^\s+(\S+)', a_line)
                if field_key:
                    custom_params['FIELDS'][field_key[0]] = {}

                # Capture invalid/empty paramater key values
                try:
                    # Possible global header containing floor params
                    if not re.match(r'^\s', a_line) and words.intersection(zero_params):
                        custom_params['ZERO'] = self.getParamValues(
                            words.intersection(zero_params).pop(), a_line)[0]

                    # Possible global header containing tolerance params
                    if not re.match(r'^\s', a_line) and words.intersection(tolerance_params):
                        custom_params['RELATIVE'] = self.getParamValues(
                            words.intersection(tolerance_params).pop(), a_line)[0]

                    # Possible field containing floor params
                    if field_key and words.intersection(zero_params):
                        custom_params['FIELDS'][field_key[0]]['ZERO'] = self.getParamValues(
                            words.intersection(zero_params).pop(), a_line)[0]

                    # Possible field containing tolerance params
                    if field_key and words.intersection(tolerance_params):
                        custom_params['FIELDS'][field_key[0]]['RELATIVE'] = self.getParamValues(
                            words.intersection(tolerance_params).pop(), a_line)[0]

                except IndexError:
                    self.setStatus(self.fail, 'comparison file syntax error')

        return custom_params
