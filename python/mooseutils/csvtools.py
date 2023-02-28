#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os, re, math, sys, argparse
from decimal import Decimal

class CSVTools:
    def __init__(self):
        self.__num_errors = 0
        self.__msg = []

    def addError(self, f, message):
        """Add error message and increment the error count"""
        self.__msg.append('In file ' + os.path.basename(f.name) + ': ' + message)
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
            text = re.sub( r'\n\s*\n', '\n', text).strip()

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
                        self.addError(f, "Number of columns ("+str(len(vals))+") not the same as number of column names ("+str(len(headers))+") in row "+repr(row))
                        break
                    for header, val in zip(headers,vals):
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
        return re.findall(param + "\s+([0-9e.\-\+]+)", param_line)

    def parseComparisonFile(self, config_file):
        """ Walk through comparison file and populate/return a dictionary as best we can """
        # A set of known paramater naming conventions. The comparison file can have these set, and we will use them.
        zero_params = set(['floor', 'abs_zero', 'absolute'])
        tolerance_params = set(['relative', 'rel_tol'])
        custom_params = {'RELATIVE' : 0.0, 'ZERO' : 0.0, 'FIELDS' : {} }

        config_file.seek(0)
        for a_line in config_file:
            s_line = a_line.strip()
            words = set(a_line.split())

            # Ignore this line if commented, is the 'time steps' header, or contains logical nots
            if s_line and \
               (s_line.startswith("#") \
                or s_line.lower().find('time steps') == 0 \
                or s_line[0] == "!"):
                continue

            field_key = re.findall(r'^\s+(\S+)', a_line)
            if field_key:
                custom_params['FIELDS'][field_key[0]] = {}

            # Capture invalid/empty paramater key values
            try:
                # Possible global header containing floor params
                if not re.match(r'^\s', a_line) and words.intersection(zero_params):
                    custom_params['ZERO'] = self.getParamValues(words.intersection(zero_params).pop(), a_line)[0]

                # Possible global header containing tolerance params
                if not re.match(r'^\s', a_line) and words.intersection(tolerance_params):
                    custom_params['RELATIVE'] = self.getParamValues(words.intersection(tolerance_params).pop(), a_line)[0]

                # Possible field containing floor params
                if field_key and words.intersection(zero_params):
                    custom_params['FIELDS'][field_key[0]]['ZERO'] = self.getParamValues(words.intersection(zero_params).pop(), a_line)[0]

                # Possible field containing tolerance params
                if field_key and words.intersection(tolerance_params):
                    custom_params['FIELDS'][field_key[0]]['RELATIVE'] = self.getParamValues(words.intersection(tolerance_params).pop(), a_line)[0]

            except IndexError:
                self.addError(config_file, "Error parsing comparison file on line: \n%s" % (a_line))

        return custom_params

