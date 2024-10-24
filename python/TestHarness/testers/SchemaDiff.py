#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from RunApp import RunApp
from TestHarness import util
import os

class SchemaDiff(RunApp):
    @staticmethod

    def validParams():
        params = RunApp.validParams()
        params.addParam('schemadiff',   [], "A list of XML or JSON files to compare.")
        params.addParam('gold_dir',      'gold', "The directory where the \"gold standard\" files (the expected output to compare against) reside relative to the TEST_DIR: (default: ./gold/). This only needs to be set if the gold file is in a non-standard location.")
        params.addParam('gold_file',      None, "Specify the file in the gold_dir that the output should be compared against. This only needs to be set if the gold file uses a different file name than the output file.")
        params.addParam('ignored_items',  [], "Items in the schema that the differ will ignore. These can be keys or values, i.e. for \"foo\": \"bar\", either foo or bar can be chosen to be selected. Note that entering a value located inside a list will skip the whole list.")
        params.addParam('rel_err',       5.5e-6, "Relative error value allowed in comparisons. If rel_err value is set to 0, it will work on the absolute difference between the values.")
        params.addParam('abs_zero',      1e-10, "Absolute zero cutoff used in diff comparisons. Every value smaller than this threshold will be ignored."),
        return params

    def __init__(self, name, params):
        RunApp.__init__(self, name, params)
        if self.specs['required_python_packages'] is None:
            self.specs['required_python_packages'] = 'deepdiff>=6.1.0'
        elif 'deepdiff' not in self.specs['required_python_packages']:
            self.specs['required_python_packages'] += ' deepdiff>=6.1.0'
        if 'packaging' not in self.specs['required_python_packages']:
            self.specs['required_python_packages'] += ' packaging'

        # So that derived classes can internally pass skip regex paths
        self.exclude_regex_paths = []

    def getOutputFiles(self, options):
        return self.specs['schemadiff']

    def prepare(self, options):
        if self.specs['delete_output_before_running'] == True:
            util.deleteFilesAndFolders(self.getTestDir(), self.getOutputFiles(options))

    def processResults(self, moose_dir, options, exit_code, runner_output):
        output = super().processResults(moose_dir, options, exit_code, runner_output)

        specs = self.specs

        if self.isFail() or specs['skip_checks']:
            return output

        # Don't run on scaled tests
        if options.scaling and specs['scale_refine']:
            return output

        # Loop over every file
        for file in specs['schemadiff']:
            gold_file = specs['gold_file'] if specs['gold_file'] else file
            if not os.path.exists(os.path.join(self.getTestDir(), specs['gold_dir'], gold_file)):
                output += "File Not Found: " + os.path.join(self.getTestDir(), specs['gold_dir'], file)
                self.setStatus(self.fail, 'MISSING GOLD FILE')
                break

            # Perform diff
            else:
                gold = os.path.join(self.getTestDir(), specs['gold_dir'], gold_file)
                test = os.path.join(self.getTestDir(), file)
                # We always ignore the header_type attribute, since it was
                # introduced in VTK 7 and doesn't seem to be important as
                # far as Paraview is concerned.
                specs['ignored_items'].append('header_type')

                gold_dict = self.try_load(gold)
                test_dict = self.try_load(test)

                if isinstance(gold_dict, Exception):
                    output += "Gold Schema File Failed To Load: "+gold+"\n"
                    output += "Gold Schema Exception: " + str(gold_dict) + "\n"
                    self.setStatus(self.fail, 'LOAD FAILED')

                if isinstance(test_dict, Exception):
                    output += "Test Schema File Failed To Load: "+test+"\n"
                    output += "Test Schema Exception: " + str(test_dict) + "\n"
                    self.setStatus(self.fail, 'LOAD FAILED')

                #Break after testing both to provide both errors in the log if both are applicable
                if isinstance(test_dict, Exception) or isinstance(gold_dict, Exception):
                    break

                # Perform the diff.
                diff = self.do_deepdiff(gold_dict, test_dict, specs['rel_err'], specs['abs_zero'], specs['ignored_items'])
                if diff:
                    output += "Schema difference detected.\nFile 1: " + gold + "\nFile 2: " + test + "\nErrors:\n"
                    output += diff
                    self.setStatus(self.diff, 'SCHEMADIFF')
                    break
        return output

    def do_deepdiff(self,orig, comp, rel_err, abs_zero, exclude_values:list=None):
        import deepdiff
        from deepdiff.operator import BaseOperator
        from packaging.version import Version

        class testcompare(BaseOperator):
            def __init__(self, rel_err,abs_zero,types,regex_paths=None):
                self.rel_err = rel_err
                self.abs_zero = abs_zero
                #next two members are necessary for deepdiff constructor to work
                self.regex_paths = regex_paths
                self.types = types

            def give_up_diffing(self,level, diff_instance):
                try:
                    if level.t1 != level.t2:
                        x = float(level.t1)
                        y = float(level.t2)

                        if x < self.abs_zero and y < self.abs_zero:
                            return True
                        if self.rel_err == 0 or y == 0:
                            if abs(x-y) > self.rel_err:
                                return False
                        elif abs(x-y)/y > self.rel_err:
                            return False
                    return True #if the two items are the same, you can stop evaluating them.

                #Often in XML data is not stored correctly as a list/array, and are instead big strings. This should be fixed with "fix_XML_arrays",
                #but here we do the logic to diff the long string in case it sneaks in, or if for some reason, someone made a JSON like this.
                except ValueError:
                    try:
                        split1 = level.t1.split(" ")
                        split2 = level.t2.split(" ")
                        if len(split1) != len(split2):
                            return False
                        for i in range(len(split1)):
                            if not split1[i] and not split2[i]: #if the two values are both just an empty str, continue.
                                continue
                            x = float(split1[i])
                            y = float(split2[i])
                            if x != y:
                                if x < self.abs_zero and y < self.abs_zero:
                                    return True
                                if self.rel_err == 0 or y == 0:
                                    if abs(x-y) > self.rel_err:
                                        return False
                                elif abs(x-y)/y > self.rel_err:
                                    return False
                        return True #if the values in the pseudo-list are different, but all fall within the accepted rel_err, the list is skipped for diffing.
                    except ValueError:
                        return False
        exclude_paths = []
        if exclude_values:
            for value in exclude_values:
                search = orig | deepdiff.search.grep(value, case_sensitive=True)
                search2 = comp | deepdiff.search.grep(value, case_sensitive=True)
                if search:
                    for path in search["matched_paths"]:
                        exclude_paths.append(path)
                if search2:
                    for path in search2["matched_paths"]:
                        exclude_paths.append(path)

        custom_operators = [testcompare(types=[str,float],rel_err=rel_err,abs_zero=abs_zero)]
        args = [orig, comp]
        kwargs = {'exclude_paths': exclude_paths,
                  'exclude_regex_paths': self.exclude_regex_paths,
                  'custom_operators': custom_operators}

        # 8.0.0 introduces and sets threshold_to_diff_deeper=0.33; this reverts
        # to the previous behavior. If a dict is different enough, it'll report
        # the whole thing as different instead of listing a large amount of keys
        # that are different. We would prefer to see all of the individual
        # keys that are different. It also seems like the "significant difference"
        # measure is done before things are excluded, which doesn't work with
        # how much we exclude from the system information by default. This might
        # be a bug, but I'm not sure. Even if it is a bug, we still want to see
        # each key individually as being different instead of the whole thing
        # being different.
        deepdiff_version = Version(deepdiff.__version__)
        if deepdiff_version >= Version('8.0.0'):
            kwargs['threshold_to_diff_deeper'] = 0

        return deepdiff.DeepDiff(*args, **kwargs).pretty()

    #this is how we call the load_file in the derived classes, and also check for exceptions in the load
    #all python functions are virtual, so there is no templating, but some self shenanigans required
    def try_load(self, path1):
        try:
            return self.load_file(path1)
        except Exception as e:
            return e
