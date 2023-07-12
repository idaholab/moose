#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import re
from functools import reduce  # Dictionary transversal
import operator               # Dictionary transversal
from FileTester import FileTester
import moosetree

try:
    import deepdiff
    from mooseutils.deepdiff_custom_operator import CompareDiff
except:
    pass

class SchemaDiff(FileTester):
    """ Generic Differntial Tester """
    @staticmethod
    def validParams():
        params = FileTester.validParams()
        params.addParam('schemadiff',       [], 'A list of CSV, XML, or JSON files to compare.')
        params.addParam('gold_file',      None, 'Specify the file in the gold_dir that the output '
                                                'should be compared against. This only needs to '
                                                'be set if the gold file uses a different file '
                                                'name than the output file.')
        params.addParam('ignored_items',    [], 'Regular expressions of items to ignore')
        params.addParam('abs_err',      5.5e-6, 'Absolute error value to be used in comparisons.')

        return params

    def __init__(self, name, params):
        """ Initialize SchemaDiff """
        # Convert possible test spec files declaring a string representation to a float
        params['abs_err'] = float(params['abs_err'])
        FileTester.__init__(self, name, params)
        if self.specs['required_python_packages'] is None:
            self.specs['required_python_packages'] = 'deepdiff>=6.1.0'
        elif 'deepdiff' not in self.specs['required_python_packages']:
            self.specs['required_python_packages'] += ' deepdiff>=6.1.0'

        # So that derived classes can internally pass skip regex paths
        self.exclude_regex_paths = []

    def getOutputDirs(self, options):
        return self.specs['schemadiff']

    def processResults(self, moose_dir, options, output):
        output += self.testFileOutput(moose_dir, options, output)
        self.testExitCodes(moose_dir, options, output)
        specs = self.specs

        if self.isFail() or specs['skip_checks']:
            return output

        # Don't run on scaled tests
        if options.scaling and specs['scale_refine']:
            return output

        # Support unittests. Explain what post-process is taking place
        output += f'Running {specs["type"].lower()}\n'

        # Loop over every file
        for file in specs['schemadiff']:
            gold_file = specs['gold_file'] if specs['gold_file'] else file
            abs_gold_file = os.path.join(self.getTestDir(), specs['gold_dir'], gold_file)
            abs_test_file = os.path.join(self.getTestDir(), file)

            if not os.path.exists(abs_gold_file):
                output += f'File Not Found: {abs_gold_file}'
                self.setStatus(self.fail, 'MISSING GOLD FILE')
                break

            # Perform diff
            else:
                # We always ignore the header_type attribute, since it was
                # introduced in VTK 7 and doesn't seem to be important as
                # far as Paraview is concerned.
                specs['ignored_items'].append('header_type')

                gold_dict = self.try_load(abs_gold_file)
                test_dict = self.try_load(abs_test_file)

                if isinstance(gold_dict, Exception):
                    output += (f'Test Schema File Failed To Load: {abs_gold_file}\n'
                               f'Gold Schema Exception: {str(gold_dict)}\n')

                if isinstance(test_dict, Exception):
                    output += (f'Test Schema File Failed To Load: {abs_test_file}\n'
                               f'Gold Schema Exception: {str(test_dict)}\n')

                #Break after testing both to provide both errors in the log if both are applicable
                if isinstance(test_dict, Exception) or isinstance(gold_dict, Exception):
                    self.setStatus(self.fail, 'LOAD FAILED')
                    break

                # Perform the diff.
                (diff, exception) = self.do_deepdiff(gold_dict,
                                                     test_dict,
                                                     specs['rel_err'],
                                                     specs['abs_err'],
                                                     specs['abs_zero'],
                                                     specs['ignored_items'])

                if exception:
                    output += f'\nDeepDiff Exception:\n{exception}\n'
                    self.setStatus(self.fail, 'DeepDiff exception')
                    return output

                if diff:
                    output += (f'Gold File: {abs_gold_file}\n'
                               f'Results File: {abs_test_file}\n{diff}\n')
                    self.setStatus(self.diff, f'{self.specs["type"].upper()}')
                    break
        return output

    def format_diff(self, diff_dict, reference, result):
        """ Iterate over known problem keys to format pretty results """
        formated_results = ''
        for key in diff_dict.keys():
            # Different Values
            if key == 'values_changed':
                formated_results += '\nValue Diff:'
                for diff_key, key_value in diff_dict['values_changed'].items():
                    formated_results += (f'\n{diff_key.replace("root", "", 1)}'
                                         f'\nGOLD:   {key_value["old_value"]}'
                                         f'\nRESULT: {key_value["new_value"]}')
            # Different fields added
            if key == 'iterable_item_added':
                formated_results += '\nAdditional List Item:'
                for diff_key, key_value in diff_dict['iterable_item_added'].items():
                    formated_results += (f'\nAdditional row at {diff_key.replace("root", "", 1)}:'
                                         f'\t{key_value}')
            # Different fields removed
            if key == 'iterable_item_removed':
                formated_results += '\nMissing List Item:'
                for diff_key, key_value in diff_dict['iterable_item_removed'].items():
                    formated_results += (f'\n{diff_key.replace("root", "", 1)}: {key_value}')

            if key == 'dictionary_item_added':
                formated_results += '\nAdditional Key:'
                for added_key in diff_dict['dictionary_item_added']:
                    formated_results += (f'\t{added_key.replace("root", "", 1)}\n')

        # custom operator caught a diff
        if 'diff' in diff_dict:
            if 'root' in diff_dict['diff']:
                formated_results += f'{diff_dict["diff"]["root"]}'
            # Multiple root entries
            else:
                formated_results += '\n\nNode Differences:'
                find_nodes = re.compile(r'\[\'|([\w+:@#]+)|\'\]')
                for node_key, value in diff_dict['diff'].items():
                    nodes = list(filter(None, find_nodes.findall(node_key)))
                    str_node = node_key.replace("root", "", 1)
                    gold = self.get_dictitem(reference, nodes[1:])
                    computed = self.get_dictitem(result, nodes[1:])
                    formated_results += (f'\nGOLD:   {str_node}: {gold}'
                                         f'\nRESULT: {str_node}: {computed}')

        return formated_results

    def compare_func(self, x, y, level):
        """
        Our custom iterator. Needed for what we believe is a bug
        See issue: https://github.com/seperman/deepdiff/issues/404
        """
        return True

    def do_deepdiff(self, orig, comp, rel_err, abs_err, abs_zero, exclude_values:list=None):
        diff = ''
        exclude_paths = []
        generic_error = None
        if exclude_values:
            for value in exclude_values:
                search = orig | deepdiff.search.grep(value, case_sensitive=True)
                search2 = comp | deepdiff.search.grep(value, case_sensitive=True)
                if search:
                    for path in search['matched_paths']:
                        exclude_paths.append(path)
                if search2:
                    for path in search2['matched_paths']:
                        exclude_paths.append(path)

        custom_operators = [CompareDiff(types=[str,float],
                                        rel_err=rel_err,
                                        abs_err=abs_err,
                                        abs_zero=abs_zero)]
        try:
            diff = deepdiff.DeepDiff(orig,
                                     comp,
                                     exclude_paths=exclude_paths,
                                     exclude_regex_paths=self.exclude_regex_paths,
                                     custom_operators=custom_operators,
                                     iterable_compare_func=self.compare_func)

        except Exception as generic_error:
            return (diff, generic_error)

        # return friendly readable results where we can
        if len(diff.affected_paths) or len(diff.affected_root_keys) or 'diff' in diff:
            return (self.format_diff(diff, orig, comp), generic_error)

        # Empty when there is no diff
        return (diff.pretty(), generic_error)

    @staticmethod
    def get_dictitem(dict_data, map_list):
        """ return a value nested inside a dictionary """
        return reduce(operator.getitem,
                      [int(x) if x.isdigit() else x for x in map_list],
                      dict_data)

    # this is how we call the load_file in the derived classes, and also check for exceptions in the
    # load all python functions are virtual, so there is no templating, but some self shenanigans
    # required
    def try_load(self, path1):
        try:
            return self.load_file(path1)
        except Exception as e:
            return e
