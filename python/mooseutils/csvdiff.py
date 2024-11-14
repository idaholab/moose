#!/usr/bin/env python3
# * This file is part of the MOOSE framework
# * https://www.mooseframework.org
# *
# * All rights reserved, see COPYRIGHT for full restrictions
# * https://github.com/idaholab/moose/blob/master/COPYRIGHT
# *
# * Licensed under LGPL 2.1, please see LICENSE for details
# * https://www.gnu.org/licenses/lgpl-2.1.html

import os
import re
import math
import sys
import argparse
from decimal import Decimal


class CSVTools:
    def __init__(self):
        self.__num_errors = 0
        self.__msg = []

    def addError(self, f, message):
        """Add error message and increment the error count"""
        self.__msg.append(
            'In file ' + os.path.basename(f.name) + ': ' + message)
        self.__num_errors += 1
        return self.__msg

    def addMessage(self, message):
        """Add a message to be printed before exit"""
        self.__msg.append(message)

    def getMessages(self):
        """Return stored messages"""
        return self.__msg

    def getNumErrors(self):
        """Return number of errors"""
        return self.__num_errors

    def convertToTable(self, files):
        """Convert text to a map of column names to column values"""
        table_pair = []
        for f in files:
            f.seek(0)
            text = f.read()
            text = re.sub(r'\n\s*\n', '\n', text).strip()

            # Exceptions occur if you try to parse a .e file
            try:
                lines = text.split('\n')
                headers = [x.strip() for x in lines.pop(0).split(',')]
                table = {}
                for header in headers:
                    table[header] = []

                for row in lines:
                    vals = row.split(',')
                    if len(headers) != len(vals):
                        self.addError(f, "Number of columns ("+str(len(vals)) +
                                      ") not the same as number of column names ("+str(len(headers))+") in row "+repr(row))
                        break
                    for header, val in zip(headers, vals):
                        try:
                            table[header].append(float(val))
                        except:
                            # ignore strings
                            table[header].append(0)

            except Exception as e:
                self.addError(f, "Exception parsing file: "+str(e.args))
                return {}

            table_pair.append(table)

        return table_pair

    def getParamValues(self, param, param_line):
        """ return a list of discovered values for param """
        return re.findall(param + r"\s+([0-9e.\-\+]+)", param_line)

    def parseComparisonFile(self, config_file):
        """ Walk through comparison file and populate/return a dictionary as best we can """
        # A set of known paramater naming conventions. The comparison file can have these set, and we will use them.
        zero_params = set(['floor', 'abs_zero', 'absolute'])
        tolerance_params = set(['relative', 'rel_tol'])
        custom_params = {'RELATIVE': 0.0, 'ZERO': 0.0, 'FIELDS': {}}

        config_file.seek(0)
        for a_line in config_file:
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
                self.addError(
                    config_file, "Error parsing comparison file on line: \n%s" % (a_line))

        return custom_params


class CSVSummary(CSVTools):
    def __init__(self, args):
        CSVTools.__init__(self)
        self.files = args.summary
        self.abs_zero = float(args.abs_zero)
        self.rel_tol = float(args.relative_tolerance)

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        for a_file in self.files:
            a_file.close()

    def _getMinMax(field_list):
        """ return the minimum and maximum values in the list along with the row id they were discovered on """
        return

    def summary(self):
        """ Parse the CSV file and generate a summary """
        table1 = self.convertToTable(self.files)[0]

        if self.getNumErrors():
            return self.getMessages()

        formatted_messages = ['GLOBAL VARIABLES relative %s floor %s' % (
            self.rel_tol, self.abs_zero)]

        field_len = []
        value_len = []
        for field, value in table1.items():
            field_len.append(len(field))
            value_len.append(len("# min: %.3e @ t%d" %
                             (Decimal(min(value)), value.index(min(value)))))

        field_justify = max(field_len) + 10
        value_justify = max(value_len) + 5

        # Generate a 'TIME STEPS' summary line if a time field does not exist. This is to maintain compliance with an exodiff summary
        if 'time' not in [x.lower() for x in table1.keys()]:
            value_count = len(table1[list(table1.keys())[0]]) - 1
            formatted_messages.insert(
                0, 'TIME STEPS relative 1 floor 0  # min: 0 @ t0  max: %d @ t%d\n' % (value_count, value_count))

        for field, value in table1.items():
            if field.lower() == 'time':
                # Tolerance for time steps will be the same for value tolerances for now (future csvdiff capability will separate this tolerance)
                formatted_messages.insert(0, 'TIME STEPS relative %s floor %s  # min: %d @ t%d  max: %d @ t%d\n' %
                                          (self.rel_tol, self.abs_zero, min(value), value.index(min(value)), max(value), value.index(max(value))))

            formatted_messages.append('%s%s%s# min: %.3e @ t%d%smax: %.3e @ t%d' %
                                      (" "*4,
                                       field,
                                       " "*((field_justify - len(field)) + 10),
                                       Decimal(min(value)), value.index(
                                           min(value)),
                                       " "*((value_justify - len("# min: %.3e @ t%d" %
                                            (Decimal(min(value)), value.index(min(value))))) + 5),
                                       Decimal(max(value)), value.index(max(value))))

        self.addMessage('\n'.join(formatted_messages))
        return self.getMessages()


class CSVDiffer(CSVTools):
    def __init__(self, args):
        CSVTools.__init__(self)
        self.files = args.csv_file
        self.config = args.comparison_file
        self.abs_zero = float(args.abs_zero)
        self.rel_tol = float(args.relative_tolerance)
        self.custom_columns = args.custom_columns
        self.custom_rel_err = args.custom_rel_err
        self.custom_abs_zero = args.custom_abs_zero
        self.ignore = args.ignore_fields
        if (not self.ignore):
            self.ignore = []
        self.__only_compare_custom = False

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        for a_file in self.files:
            a_file.close()

        if self.config:
            self.config.close()

    # diff the files added to the system and return a message of differences
    # This method should only be called once. If it called again you must
    # manually clear messages by calling clearDiff
    def diff(self):
        abs_zero = self.abs_zero
        rel_tol = self.rel_tol

        # Setup custom values based on supplied config file. Override any information
        # in self.custom_colums (indeed, verifyArgs will not allow both --custom and
        # --config to be used together anyway)
        if self.config:
            self.__only_compare_custom = True
            custom_params = self.parseComparisonFile(self.config)
            abs_zero = custom_params.get('ZERO', abs_zero)
            rel_tol = custom_params.get('RELATIVE', rel_tol)
            if self.getNumErrors():
                return self.getMessages()

            self.custom_columns = []
            self.custom_rel_err = []
            self.custom_abs_zero = []

            for field_id, value in custom_params['FIELDS'].items():
                self.custom_columns.append(field_id)
                self.custom_abs_zero.append(value.get('ZERO', abs_zero))
                self.custom_rel_err.append(value.get('RELATIVE', rel_tol))

        # Setup data structures for holding customized relative tolerance and absolute
        # zero values and flag for checking variable names
        rel_err_map = {}
        abs_zero_map = {}
        found_column = {}
        if self.custom_columns:
            for i in range(0, len(self.custom_columns)):
                rel_err_map[self.custom_columns[i]] = float(
                    self.custom_rel_err[i])
                abs_zero_map[self.custom_columns[i]] = float(
                    self.custom_abs_zero[i])
                found_column[self.custom_columns[i]] = False

        # use this value to skip the rest of the tests when we've found an error
        # the order of the tests is most general to most specific, so if a general
        # one fails then the more specific ones will probably not only fail, but
        # crash the program because it's looking in a column that doesn't exist
        (table1, table2) = self.convertToTable(self.files)

        # Make sure header names are the same (also makes sure # cols is the same)
        # This way it reports what column is missing, not just # cols is different
        keys1 = table1.keys()
        keys2 = table2.keys()
        (large, small) = (keys1, keys2)

        # check if custom tolerances used, column name exists in one of
        # the CSV files
        if self.custom_columns and self.__only_compare_custom:
            # For the rest of the comparison, we only care about custom columns
            keys1 = self.custom_columns
            for key in self.custom_columns:
                if key not in small or key not in large:
                    self.addError(
                        self.files[0], "Header '" + key + "' is missing")
        elif len(keys1) < len(keys2):
            (large, small) = (keys2, keys1)
            for key in large:
                if key not in small and key not in self.ignore:
                    self.addError(
                        self.files[0], "Header '" + key + "' is missing")
        elif len(keys1) > len(keys2):
            for key in large:
                if key not in small and key not in self.ignore:
                    self.addError(
                        self.files[1], "Header '" + key + "' is missing")
        else:
            for key in keys1:
                found_column[key] = True

        if self.getNumErrors():
            return self.getMessages()

        # now check that each column is the same length
        for key in keys1:
            if key not in self.ignore and len(table1[key]) != len(table2[key]):
                self.addError(
                    self.files[0], "Columns with header '" + key + "' aren't the same length")
                # assume all columns are the same length, so don't report the other errors
                break

        if self.getNumErrors():
            return self.getMessages()

        # now check all the values in the table
        for key in keys1:
            if key in self.ignore:
                continue
            for val1, val2 in zip(table1[key], table2[key]):
                # if customized tolerances specified use them otherwise
                # use the default
                if self.custom_columns:
                    try:
                        abs_zero = abs_zero_map[key]
                    except:
                        abs_zero = self.abs_zero
                if abs(val1) < abs_zero:
                    val1 = 0
                if abs(val2) < abs_zero:
                    val2 = 0

                # disallow nan in the gold file
                if math.isnan(val1):
                    self.addError(
                        self.files[0], "The values in column \"" + key.strip() + "\" contain NaN")

                # disallow inf in the gold file
                if math.isinf(val1):
                    self.addError(
                        self.files[0], "The values in column \"" + key.strip() + "\" contain Inf")

                # if they're both exactly zero (due to the threshold above) then they're equal so pass this test
                if val1 == 0 and val2 == 0:
                    continue

                rel_diff = 0
                if max(abs(val1), abs(val2)) > 0:
                    rel_diff = abs((val1 - val2) / max(abs(val1), abs(val2)))

                # if customized tolerances specified use them otherwise
                # use the default
                if self.custom_columns:
                    try:
                        rel_tol = rel_err_map[key]
                    except:
                        rel_tol = self.rel_tol
                if rel_diff > rel_tol:
                    self.addError(self.files[1], "The values in column \"%s\" don't match. \n\trelative diff:   %.3e ~ %.3e = %.3e (%.3e)" % (key.strip(),
                                                                                                                                              val1,
                                                                                                                                              val2,
                                                                                                                                              rel_diff,
                                                                                                                                              Decimal(rel_diff)))
                    # assume all other vals in this column are wrong too, so don't report them
                    break

        # Loop over variable names to check if any are missing from all the
        # CSV files being compared
        if self.custom_columns and not self.__only_compare_custom:
            for mykey2 in self.custom_columns:
                if not found_column[mykey2]:
                    self.addError(
                        self.files[0], "all CSV files Variable '" + mykey2 + "' in custom_columns is missing")

        return self.getMessages()


def verifyArgs(args):
    problems = []
    if not args.csv_file and not args.summary:
        problems.append('No input files given')

    elif len(args.csv_file) != 2 and not args.summary:
        problems.append('Specify two files to compare')

    elif args.csv_file and args.summary:
        problems.append(
            'Incorrect positional arguments, or you are trying to perform a diff and show a summary (can only do one or the other)')

    elif args.summary and args.comparison_file:
        print(
            'Ignoring request to use config file while being asked to display a summary\n')

    # Check if all custom args are populated correctly
    unify_custom_args = [x for x in [args.custom_columns,
                                     args.custom_abs_zero, args.custom_rel_err] if x != None]
    if unify_custom_args and len(unify_custom_args) != 3:
        problems.append(
            'When using any --custom-* option, you must use all three')
    elif unify_custom_args:
        if len(set([len(x) for x in unify_custom_args])) > 1:
            problems.append(
                'All --custom-* options need to contain the same number of space separated items')

    if unify_custom_args and args.comparison_file:
        problems.append(
            'When supplying a config file (--comparison-file|-c), you can not use any --custom-* args')

    for a_problem in problems:
        print(a_problem)
    if problems:
        sys.exit(1)

    return args


def parseArgs(args=None):
    parser = argparse.ArgumentParser(
        description='Tool for testing differences between two CSV files')
    parser.add_argument('csv_file', nargs='*', type=argparse.FileType('r'))
    parser.add_argument('--summary', '-s', nargs=1, type=argparse.FileType('r'),
                        metavar='csv_file', help='Produce a summary in csvdiff input format')
    parser.add_argument('--comparison-file', '-c', type=argparse.FileType('r'), metavar='comparison_file',
                        help='Use comparison configuration file (can be generated using --summary|-s)')
    parser.add_argument('--ignore-fields', '-i', nargs='+', metavar='id',
                        help='Ignore specified space-separated field IDs')
    parser.add_argument('--diff-fields', '-f', nargs='+', metavar='id',
                        help='Perform diff tests only on space-separated field IDs')
    parser.add_argument('--abs-zero', metavar='absolute zero', default='1e-11',
                        help='Value representing an absolute zero (default: 1e-11)')
    parser.add_argument('--relative-tolerance', metavar='tolerance', default='5.5e-6',
                        help='Value representing the acceptable tolerance between comparisons (default: 5.5e-6)')
    parser.add_argument('--custom-columns', nargs='+', metavar='field',
                        help='Space separated list of custom field IDs to compare')
    parser.add_argument('--custom-abs-zero', nargs='+', metavar='exponential',
                        help='Space separated list of corresponding exponential absolute zero values for --custom-colums')
    parser.add_argument('--custom-rel-err', nargs='+', metavar='exponential',
                        help='Space separated list of corresponding acceptable exponential tolerance values for --custom-colums')
    return verifyArgs(parser.parse_args(args))


if __name__ == '__main__':
    args = parseArgs()
    if args.summary:
        with CSVSummary(args) as csv_summary:
            messages = csv_summary.summary()
            errors = csv_summary.getNumErrors()

    else:
        with CSVDiffer(args) as csv_differ:
            messages = csv_differ.diff()
            errors = csv_differ.getNumErrors()

    for a_message in messages:
        print(a_message)

    if not errors and not args.summary:
        print("Files are the same")

    if errors:
        sys.exit(1)
