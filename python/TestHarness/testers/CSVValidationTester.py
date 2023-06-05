#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from FileTester import FileTester
from TestHarness import util
from mooseutils.csvdiff import CSVTools
from mooseutils import colorText
import os
import math
import statistics

def diff_files(gold_file, out_file, err_type='relative'):
    """
    Diff 2 CSV files and compute mean and std. deviation from all values

    'time' column is ignored
    """

    found_column = {}

    csv = CSVTools()
    (table1, table2) = csv.convertToTable([
        open(gold_file, 'r'),
        open(out_file, 'r')
    ])

    # Make sure header names are the same (also makes sure # cols is the same)
    # This way it reports what column is missing, not just # cols is different
    keys1 = table1.keys()
    keys2 = table2.keys()
    (large, small) = (keys1, keys2)

    if len(keys1) < len(keys2):
        (large, small) = (keys2, keys1)
        for key in large:
            if key not in small:
                csv.addError(gold_file, "Header '" + key + "' is missing")
    elif len(keys1) > len(keys2):
        for key in large:
            if key not in small:
                csv.addError(out_file, "Header '" + key + "' is missing")
    else:
        for key in keys1:
            found_column[key] = True

    if csv.getNumErrors():
        return csv.getMessages()

    # now check that each column is the same length
    for key in keys1:
        if len(table1[key]) != len(table2[key]):
            csv.addError(self.files[0], "Columns with header '" + key + "' aren't the same length")
            # assume all columns are the same length, so don't report the other errors
            break

    if csv.getNumErrors():
        return csv.getMessages()

    # now check all the values in the table
    a = []
    for key in keys1:
        if key == 'time':
            continue
        for val1, val2 in zip(table1[key], table2[key]):
            # disallow nan in the gold file
            if math.isnan(val1):
                csv.addError(gold_file, "The values in column \"" + key.strip() + "\" contain NaN")

            # disallow inf in the gold file
            if math.isinf(val1):
                csv.addError(gold_file, "The values in column \"" + key.strip() + "\" contain Inf")

            if err_type == 'relative':
                diff = (val2 - val1) / val1
            else:
                diff = val2 - val1
            a.append(diff)

    mean = statistics.mean(a)
    std = statistics.stdev(a)
    return (mean, std)


class CSVValidationTester(FileTester):
    """
    Compares a CSV file produced by a MOOSE-based application against a CSV file with measured data

    Computes mean and standard deviation from relative or absolute errors.
    """

    @staticmethod
    def validParams():
        params = FileTester.validParams()
        params.addRequiredParam('csvdiff', [], "A list of files that will be compared.")
        params.addParam('err_type', 'relative', "Type of error ('relative' or 'absolute').")
        params.addRequiredParam('mean_limit', "Requested mean value.")
        params.addRequiredParam('std_limit', "Requested standard deviation value.")
        return params

    def __init__(self, name, params):
        FileTester.__init__(self, name, params)
        # formatting
        self.file_name_len = 40

    def processResults(self, moose_dir, options, output):
        FileTester.processResults(self, moose_dir, options, output)

        if self.isFail() or self.specs['skip_checks']:
            return output

        # Don't Run CSVDiff on Scaled Tests
        if options.scaling and self.specs['scale_refine']:
            return output

        output = ""
        # Make sure that all of the CSVDiff files are actually available
        for file in self.specs['csvdiff']:
            if not os.path.exists(os.path.join(self.getTestDir(), self.specs['gold_dir'], file)):
                output += "File Not Found: " + os.path.join(self.getTestDir(), self.specs['gold_dir'], file)
                self.setStatus(self.fail, 'MISSING GOLD FILE')
                break

        if not self.isFail():
            output += "\n"
            fmt = "{{:{}s}} | {{:20s}} | {{:20s}}\n".format(self.file_name_len)
            output += fmt.format("file", "computed", "requested")
            output += "-" * (self.file_name_len + 46) + "\n"

            ok = True
            for file in self.specs['csvdiff']:
                gold_file = os.path.join(self.getTestDir(), self.specs['gold_dir'], file)
                out_file = os.path.join(self.getTestDir(), file)
                (mean, std) = diff_files(gold_file, out_file, self.specs['err_type'])

                computed = ""
                if mean > self.specs['mean_limit']:
                    clr = 'RED'
                    ok = False
                else:
                    clr = 'GREEN'
                computed += colorText("{:.2f}".format(mean), clr, html=False, colored=options.colored, code=options.code)
                computed += " \u00B1 "
                if std > self.specs['std_limit']:
                    clr = 'RED'
                    ok = False
                else:
                    clr = 'GREEN'
                computed += colorText("{:.2f}".format(std), clr, html=False, colored=options.colored, code=options.code)

                requested = "{:.2f} \u00B1 {:.2f}".format(self.specs['mean_limit'], self.specs['std_limit'])

                if options.colored:
                    # need to account for the color characters in the second column
                    fmt = "{{:{}s}} | {{:38s}} | {{:20s}}\n".format(self.file_name_len)
                else:
                    fmt = "{{:{}s}} | {{:20s}} | {{:20s}}\n".format(self.file_name_len)
                output += fmt.format(file, computed, requested)

            if not ok:
                self.setStatus(self.diff, 'DIFF')

        return output
