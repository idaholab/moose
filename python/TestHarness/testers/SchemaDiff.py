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
import json, os

class SchemaDiff(RunApp):
    @staticmethod
    def validParams():
        params = RunApp.validParams()
        params.addRequiredParam('schemadiff',   [], "A list of XML or JSON files to compare.")
        params.addParam('gold_dir',      'gold', "The directory where the \"golden standard\" files reside relative to the TEST_DIR: (default: ./gold/). This only needs to be set if the gold file is in a non-standard location.")
        params.addParam('gold_file',      None, "Specify the file in the gold_dir that the output should be compared against. This only needs to be set if the gold file uses a different file name than the output file.")
        params.addParam('ignored_items',  [], "Items in the schema that the differ to ignore. These can be keys or values, i.e. for \"foo\": \"bar\", either foo or bar can be chosen to be selected. Note that entering a value located inside a list will skip the whole list.")
        params.addParam('rel_err',       5.5e-6, "Relative error value allowed in comparisons. If rel_err value is set to 0, it will work on the absolute difference between the values")
        return params

    def __init__(self, name, params):
        RunApp.__init__(self, name, params)
        if self.specs['required_python_packages'] is None:
            self.specs['required_python_packages'] = 'deepdiff xmltodict'
        elif 'deepdiff' not in self.specs['required_python_packages']:
            self.specs['required_python_packages'] += ' deepdiff'
        elif 'xmltodict' not in self.specs['required_python_packages']:
            self.specs['required_python_packages'] += ' xmltodict'

    def prepare(self, options):
        if self.specs['delete_output_before_running'] == True:
            util.deleteFilesAndFolders(self.getTestDir(), self.specs['schemadiff'])
    def processResults(self, moose_dir, options, output):
        output += self.testFileOutput(moose_dir, options, output)
        self.testExitCodes(moose_dir, options, output)
        # Skip
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

                gold_dict = self.load_file(gold)
                test_dict = self.load_file(test)
                if not gold_dict:
                    output += "Gold Schema File Invalid: "+gold+"\n"
                    self.setStatus(self.fail, 'INVALID SCHEMA(S) PROVIDED')
                if not test_dict:
                    output += "Test Schema File Invalid: " + test +"\n"
                    self.setStatus(self.fail, 'INVALID SCHEMA(S) PROVIDED')

                #Break after testing both to provide both errors in the log if both are applicable
                if not test_dict or not gold_dict:
                    break
                #Output err and break if gold and test filetypes are different
                if gold_dict[1] != test_dict[1]:
                    output += "Schema Files of Different Types: \nTest: " + test +"\nGold: " + gold
                    self.setStatus(self.fail, 'INVALID SCHEMA(S) PROVIDED')
                    break
                # Get the results of the diff
                diff = self.do_deepdiff(gold_dict[0], test_dict[0], specs['rel_err'], specs['ignored_items'])
                if diff:
                    output += "Schema difference detected.\nFile 1: " + gold + "\nFile 2: " + test + "\nErrors:\n"
                    output += diff
                    self.setStatus(self.diff, 'SCHEMADIFF')
                    break

        return output

    def import_xml(self,filepath):
        import xmltodict
        with open(filepath,"r") as f:
            return xmltodict.parse(f.read())

    def import_json(self,filepath):
        with open(filepath,"r") as f:
            return json.loads(f.read())

    def do_deepdiff(self,orig, comp, rel_err, exclude_values:list=None):
        import deepdiff
        from deepdiff.operator import BaseOperator
        class testcompare(BaseOperator):
            def __init__(self, rel_err,types,regex_paths=None):
                self.rel_err = rel_err
                #next two members are necessary for deepdiff constructor to work
                self.regex_paths = regex_paths
                self.types = types
            def give_up_diffing(self,level, diff_instance):
                try:
                    if level.t1 != level.t2:
                        x = float(level.t1)
                        y = float(level.t2)
                        if abs(x-y) > self.rel_err:
                            return False
                    return True #if the two items are the same, you can stop evaluating them.
                except ValueError: #try comparing them iteratively if the schema value acts as a pseudo-list.
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
                                if abs(x-y) > self.rel_err:
                                    return False
                        return True #if the values in the pseudo-list are different, but all fall within the accepted rel_err, the list is skipped for diffing.
                    except ValueError:
                        return False
        to_exclude = []
        if exclude_values:
            for value in exclude_values:
                search = orig | deepdiff.search.grep(value, case_sensitive=True)
                if search:
                    for path in search["matched_paths"]:
                        to_exclude.append(path)

        return deepdiff.DeepDiff(orig,comp,exclude_paths=to_exclude,custom_operators=[testcompare(types=[str,float],rel_err=rel_err)]).pretty()

    def load_file(self, path1):
        try:
            return (self.import_xml(path1), 1)
        except:
            try:
                return (self.import_json(path1),2)
            except:
                return False




