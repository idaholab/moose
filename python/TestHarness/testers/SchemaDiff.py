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
import xmltodict, json, deepdiff, os

class SchemaDiff(RunApp):

    @staticmethod
    def validParams():
        params = RunApp.validParams()
        params.addRequiredParam('schemadiff',   [], "A list of XML or JSON files to compare.")
        params.addParam('gold_dir',      'gold', "The directory where the \"golden standard\" files reside relative to the TEST_DIR: (default: ./gold/). This only needs to be set if the gold file is in a non-standard location.")
        params.addParam('ignored_items',  [], "Items in the schema that the differ to ignore. These can be keys or values, i.e. for \"foo\": \"bar\", either foo or bar can be chosen to be selected.")
        return params

    def __init__(self, name, params):
        RunApp.__init__(self, name, params)

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

            # Error if gold file does not exist
            if not os.path.exists(os.path.join(self.getTestDir(), specs['gold_dir'], file)):
                output += "File Not Found: " + os.path.join(self.getTestDir(), specs['gold_dir'], file)
                self.setStatus(self.fail, 'MISSING GOLD FILE')
                break

            # Perform diff
            else:
                gold = os.path.join(self.getTestDir(), specs['gold_dir'], file)
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

                # Get the results of the diff
                diff = self.do_deepdiff(gold_dict, test_dict, specs['ignored_items'])
                if diff:
                    output += "Schema difference detected.\nFile 1: " + gold + "\nFile 2: " + test + "\nErrors:\n"
                    output += diff
                    self.setStatus(self.diff, 'SCHEMADIFF')
                    break

        return output

    def import_xml(self,filepath):
        with open(filepath,"r") as f:
            return xmltodict.parse(f.read())

    def import_json(self,filepath):
        with open(filepath,"r") as f:
            return json.loads(f.read())

    def do_deepdiff(self,orig, comp, exclude_values:list=None):
        to_exclude = []
        if exclude_values:
            for value in exclude_values:
                search = orig | deepdiff.search.grep(value, case_sensitive=True)
                if search:
                    for path in search["matched_paths"]:
                        to_exclude.append(path)

        return deepdiff.DeepDiff(orig,comp,exclude_paths=to_exclude).pretty()

    def load_file(self, path1):
        try:
            return self.import_xml(path1)
        except:
            try:
                return self.import_json(path1)
            except:
                return False
