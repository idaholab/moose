#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from SchemaDiff import SchemaDiff

class JSONDiff(SchemaDiff):
    @staticmethod
    def validParams():
        params = SchemaDiff.validParams()
        params.addRequiredParam('jsondiff',   [], "A list of JSON files to compare.")
        params.addParam('skip_keys', [],"Deprecated. Items in the JSON that the differ will ignore. This is functionally identical to ignored_items inside of SchemaDiff.")
        params.addParam('ignored_regex_items', [], "Items (with regex enabled) to ignore, separated by '/' for each level, i.e., key1/key2/.* will skip all items in ['key1']['key2']['.*']")
        params.addParam('keep_system_information', False, "Whether or not to keep the system information as part of the diff.")
        params.addParam('keep_reporter_types', False, "Whether or not to keep the MOOSE Reporter type information as part of the diff.")
        return params

    def __init__(self, name, params):
        params['schemadiff'] = params['jsondiff']
        params['ignored_items'] += params['skip_keys']

        SchemaDiff.__init__(self, name, params)
        if not params['keep_system_information']:
            self.specs['ignored_items'].extend(['app_name',
                                            'current_time',
                                            'executable',
                                            'executable_time',
                                            'moose_version',
                                            'libmesh_version',
                                            'petsc_version',
                                            'slepc_version'])
        if not params['keep_reporter_types']:
            self.specs['ignored_regex_items'].append('reporters/.*/values/.*/type')

        # Form something like root['key1']['key2']... for each entry
        for entry in self.specs['ignored_regex_items']:
            re_entry = 'root'
            for key in entry.split('/'):
                re_entry += fr"\['{key}'\]"
            self.exclude_regex_paths.append(re_entry)

    def load_file(self, path1):
        import json
        with open(path1,"r") as f:
            return json.loads(f.read())
