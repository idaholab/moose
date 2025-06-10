#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import subprocess
from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def testSchemaDiff(self):
        output = self.runTests('-i', 'schemadiff', exit_code=129).output
        self.assertRegex(output, r'test_harness\.schema_jsondiff.*?FAILED \(SCHEMADIFF\)')
        self.assertRegex(output, r'test_harness\.schema_xmldiff.*?FAILED \(SCHEMADIFF\)')
        self.assertRegex(output, r'test_harness\.schema_invalid_json.*?FAILED \(LOAD FAILED\)')
        self.assertRegex(output, r'test_harness\.schema_invalid_xml.*?FAILED \(LOAD FAILED\)')
