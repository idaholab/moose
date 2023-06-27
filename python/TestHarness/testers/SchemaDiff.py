#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
from FileTester import FileTester

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
        params.addParam('rel_err',      5.5e-6, 'Relative error value allowed in comparisons. If '
                                                'rel_err value is set to 0, it will work on the '
                                                'absolute difference between the values.')
        params.addParam('abs_err',      5.5e-4, 'Absolute error value to be used in comparisons.')
        params.addParam('abs_zero',      1e-10, 'Absolute zero cutoff used in diff comparisons. '
                                                'Every value smaller than this threshold will be '
                                                'ignored.')
        return params

    def __init__(self, name, params):
        """ Initialize SchemaDiff """
        # convert test specs entering the constraints as string, i.e. rel_err = '1.1e-2' instead
        # of rel_err = 1.1e-2
        params['rel_err'] = float(params['rel_err'])
        params['abs_err'] = float(params['abs_err'])
        params['abs_zero'] = float(params['abs_zero'])
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
                    output += (f'Difference detected.\nFile 1: {abs_gold_file}\n'
                               f'File 2: {abs_test_file}\n'
                               f'Errors:\n{diff}\n')
                    self.setStatus(self.diff, f'{self.specs["type"].upper()}')
                    break
        return output

    def format_diff(self, diff_dict):
        """ Iterate over known problem keys to format pretty results """
        formated_results = f'{self.specs["type"].upper()} Detected:\n'
        for key in diff_dict.keys():
            # Different Values
            if key == 'values_changed':
                for diff_key, key_value in diff_dict['values_changed'].items():
                    formated_results += (f'{diff_key.replace("root", "", 1)}'
                                         f'\n\tGOLD:   {key_value["old_value"]}'
                                         f'\n\tRESULT: {key_value["new_value"]}\n')
            # Different fields added
            if key == 'iterable_item_added':
                for diff_key, key_value in diff_dict['iterable_item_added'].items():
                    formated_results += (f'Additional row at {diff_key.replace("root", "", 1)}:'
                                         f'\t{key_value}\n')
            # Different fields removed
            if key == 'iterable_item_removed':
                for diff_key, key_value in diff_dict['iterable_item_removed'].items():
                    formated_results += (f'Missing item at {diff_key.replace("root", "", 1)}:'
                                         f'\t{key_value}\n')

            if key == 'dictionary_item_added':
                formated_results += (f'Additional keys found in result:\n')
                for added_key in diff_dict['dictionary_item_added']:
                    formated_results += (f'\t{added_key.replace("root", "", 1)}\n')

        return formated_results

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
                                     custom_operators=custom_operators)

        except Exception as generic_error:
            return (diff, generic_error)

        # return friendly readable results where we can
        if len(diff.affected_paths) or len(diff.affected_root_keys):
            return (self.format_diff(diff), generic_error)

        # custom operator caught a diff
        if 'diff' in diff:
            return (diff['diff']['root'], generic_error)

        # Empty when there is no diff
        return (diff.pretty(), generic_error)

    # this is how we call the load_file in the derived classes, and also check for exceptions in the
    # load all python functions are virtual, so there is no templating, but some self shenanigans
    # required
    def try_load(self, path1):
        try:
            return self.load_file(path1)
        except Exception as e:
            return e
