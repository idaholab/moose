#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from SchemaDiff import SchemaDiff
from TestHarness import util
class XMLDiff(SchemaDiff):

    @staticmethod
    def validParams():
        params = SchemaDiff.validParams()
        params.addRequiredParam('xmldiff',   [], "A list of XML files to compare.")
        params.addParam('ignored_attributes',  [], "Deprecated. Items in the JSON that the differ will ignore. This is functionally identical to ignored_items inside of SchemaDiff.")
        return params

    def __init__(self, name, params):
        params['schemadiff'] = params['xmldiff']
        params['ignored_items'] = params['ignored_attributes']
        SchemaDiff.__init__(self, name, params)
        if 'xmltodict' not in self.specs['required_python_packages']:
            self.specs['required_python_packages'] += ' xmltodict'

    def prepare(self, options):
        if self.specs['delete_output_before_running'] == True:
            util.deleteFilesAndFolders(self.getTestDir(), self.specs['xmldiff'])

    def processResults(self, moose_dir, options, output):
        return SchemaDiff.processResults(self, moose_dir, options, output)

    def load_file(self, path1):
        return (self.import_xml(path1), 1)
