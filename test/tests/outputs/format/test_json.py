#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os, sys
import subprocess
import json
import unittest
from FactorySystem import Parser
import pyhit
import mooseutils

def run_app(args=[]):
    """
    Run the app and return the output.
    Exits if the app failed to run for any reason.
    """
    proc = None
    app_name = mooseutils.find_moose_executable_recursive()
    args.insert(0, app_name)
    #  "-options_left 0" is used to stop the debug version of PETSc from printing
    # out WARNING messages that sometime confuse the json parser
    args.insert(1, "-options_left")
    args.insert(2, "0")
    cmd_line = ' '.join(args)
    try:
        proc = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    except OSError as e:
        print("Problem running '%s'\nError: %s" % (cmd_line, e))
        sys.exit(1)

    data = proc.communicate()
    stdout_data = data[0].decode("utf-8")
    if proc.returncode != 0:
        print("Failed with exit code %s" % proc.returncode)
        sys.exit(proc.returncode)
    return stdout_data

class TestJSONBase(unittest.TestCase):
    """
    Make sure the Json dump produces valid Json
    and has the expected structure
    """
    def getJsonData(self, extra=[]):
        args = ["--json"] + extra
        output = run_app(args)
        self.assertIn("**START JSON DATA**\n", output)
        self.assertIn("**END JSON DATA**\n", output)

        start_json_string = '**START JSON DATA**\n'

        start_pos = output.find('**START JSON DATA**\n')
        self.assertGreater(start_pos, -1)

        end_pos = output.find('**END JSON DATA**')
        self.assertGreater(end_pos, -1)

        output = output[start_pos + len(start_json_string):end_pos]
        data = json.loads(output)
        return data

    def check_basic_json(self, data):
        self.assertIn("Executioner", data)
        self.assertIn("BCs", data)
        bcs = data["BCs"]
        periodic = bcs["subblocks"]["Periodic"]
        self.assertEqual(periodic["star"]["subblock_types"], None)

        self.assertIn("DirichletBC", bcs["star"]["subblock_types"])
        self.assertNotIn("types", bcs)

        exe = data["Executioner"]
        self.assertIn("types", exe)
        self.assertIn("Transient", exe["types"])
        self.assertNotIn("subblock_types", exe)

        params = exe["actions"]["CreateExecutionerAction"]["parameters"]
        self.assertEqual(params["active"]["cpp_type"], "std::vector<std::string>")
        self.assertEqual(params["active"]["basic_type"], "Array:String")
        self.assertEqual(params["type"]["cpp_type"], "std::string")
        self.assertEqual(params["type"]["basic_type"], "String")

        # Preconditioning has a Preconditioning/*/* syntax which is unusual
        self.assertIn("Preconditioning", data)
        p = data["Preconditioning"]
        split = p["star"]["star"]["subblock_types"]["Split"]
        self.assertIn("petsc_options", split["parameters"])
        self.assertIn("splitting_type", split["parameters"])

        f = data["Functions"]["star"]
        self.assertIn("associated_types", f)
        self.assertEquals(["FunctionName"], f["associated_types"])
        self.assertEqual(f["subblock_types"]["ParsedFunction"]["class"], "MooseParsedFunction")
        self.assertEqual(f["subblock_types"]["ParsedFunction"]["label"], "MooseApp")

        a = data["Adaptivity"]
        i = a["subblocks"]["Indicators"]["star"]["subblock_types"]["AnalyticalIndicator"]
        self.assertIn("all", i["parameters"]["outputs"]["reserved_values"])
        self.assertIn("none", i["parameters"]["outputs"]["reserved_values"])

    def getBlockSections(self, node):
        return {c.path(): c for c in node.children(node_type=hit.NodeType.Section)}

    def getBlockParams(self, node):
        return {c.path(): c for c in node.children(node_type=hit.NodeType.Field)}

    def getInputFileFormat(self, extra=[]):
        """
        Does a dump and uses the GetPotParser to parse the output.
        """
        args = ["--disable-refcount-printing", "--dump"] + extra
        output = run_app(args)
        self.assertIn("### START DUMP DATA ###\n", output)
        self.assertIn("### END DUMP DATA ###\n", output)

        output = output.split('### START DUMP DATA ###\n')[1]
        output = output.split('### END DUMP DATA ###')[0]

        self.assertNotEqual(len(output), 0)
        root = pyhit.parse(output)
        errors = list(Parser.checkDuplicates(root))
        self.assertEqual(errors, [])
        return root


class TestFull(TestJSONBase):
    def testFullJson(self):
         """
         Some basic checks to see if some data
         is there and is in the right location.
         """
         all_data = self.getJsonData()
         self.assertIn("active", all_data["global"]["parameters"])
         data = all_data["blocks"]
         self.check_basic_json(data)
         # Make sure the default dump has test objects
         self.assertIn("ApplyInputParametersTest", data)
         self.assertEqual(data["Functions"]["star"]["subblock_types"]["PostprocessorFunction"]["label"], "MooseTestApp")


class TestNoTestObjects(TestJSONBase):
    def testNoTestObjects(self):
        # Make sure test objects are removed from the output
        all_data = self.getJsonData(["--disallow-test-objects"])
        self.assertIn("active", all_data["global"]["parameters"])
        data = all_data["blocks"]
        self.check_basic_json(data)
        self.assertNotIn("ApplyInputParametersTest", data)


class TestSearch(TestJSONBase):
    def testJsonSearch(self):
        """
        Make sure parameter search works
        """
        all_data = self.getJsonData(["initial_marker"])
        self.assertNotIn("global", all_data)
        data = all_data["blocks"]
        self.assertNotIn("Executioner", data)
        self.assertNotIn("BCs", data)
        self.assertIn("Adaptivity", data)
        self.assertEqual(len(data.keys()), 1)
        params = data["Adaptivity"]["actions"]["SetAdaptivityOptionsAction"]["parameters"]
        self.assertIn("initial_marker", params)
        self.assertEqual(len(params.keys()), 1)

        # test to make sure it matches blocks as well
        all_data = self.getJsonData(["diffusion"])
        data = all_data["blocks"]
        self.assertEqual(len(data.keys()), 2)
        self.assertIn("BadKernels", data)
        self.assertIn("Kernels", data)
        diff = data["Kernels"]["star"]["subblock_types"]["Diffusion"]
        self.assertIn("use_displaced_mesh", diff["parameters"])


class TestLineInfo(TestJSONBase):
    def testLineInfo(self):
        """
        Make sure file/line information works
        """
        all_data = self.getJsonData()
        data = all_data["blocks"]
        adapt = data["Adaptivity"]["actions"]["SetAdaptivityOptionsAction"]
        fi = adapt["file_info"]
        self.assertEqual(len(fi.keys()), 1)
        fname = list(fi)[0]
        # Clang seems to have the full path name for __FILE__
        # gcc seems to just use the path that is given on the command line, which won't include "framework"
        self.assertTrue(fname.endswith(os.path.join("src", "base", "Moose.C")), 'file "{}" found instead'.format(fname))
        self.assertGreater(fi[fname], 0)

        fi = adapt["tasks"]["set_adaptivity_options"]["file_info"]
        self.assertEqual(len(fi.keys()), 1)
        fname = list(fi)[0]
        self.assertTrue(fname.endswith(os.path.join("src", "actions", "SetAdaptivityOptionsAction.C")))
        self.assertGreater(fi[fname], 0)

class TestNoTemplate(unittest.TestCase):
    def test(self):
        output = run_app(['--json'])
        self.assertNotIn('<RESIDUAL>', output)
        self.assertNotIn('<JACOBIAN>', output)

if __name__ == '__main__':
    unittest.main(__name__, verbosity=2)
