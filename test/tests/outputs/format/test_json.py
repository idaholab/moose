#!/usr/bin/env python2
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
from FactorySystem.Parser import DupWalker
import hit

def find_app():
    """
    Find the executable to use, respecting MOOSE_DIR and METHOD
    """
    moose_dir = os.environ.get("MOOSE_DIR")
    if not moose_dir:
        p = subprocess.Popen('git rev-parse --show-cdup', stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
        p.wait()
        if p.returncode == 0:
            git_dir = p.communicate()[0]
            moose_dir = os.path.abspath(os.path.join(os.getcwd(), git_dir)).rstrip()
        else:
            print("Could not find top level moose directory. Please set the MOOSE_DIR environment variable.")
            sys.exit(1)

    app_name = os.path.join(moose_dir, "test", "moose_test-%s" % os.environ.get("METHOD", "opt"))
    return app_name

def run_app(args=[]):
    """
    Run the app and return the output.
    Exits if the app failed to run for any reason.
    """
    proc = None
    app_name = find_app()
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

class TestJSON(unittest.TestCase):
    """
    Make sure the Json dump produces valid Json
    and has the expected structure
    """
    def getJsonData(self, extra=[]):
        args = ["--json"] + extra
        output = run_app(args)
        self.assertIn("**START JSON DATA**\n", output)
        self.assertIn("**END JSON DATA**\n", output)

        output = output.split('**START JSON DATA**\n')[1]
        output = output.split('**END JSON DATA**')[0]

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


    def testJson(self):
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

    def testNoTestObjects(self):
        # Make sure test objects are removed from the output
        all_data = self.getJsonData(["--disallow-test-objects"])
        self.assertIn("active", all_data["global"]["parameters"])
        data = all_data["blocks"]
        self.check_basic_json(data)
        self.assertNotIn("ApplyInputParametersTest", data)

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
        root = hit.parse("dump.i", output)
        hit.explode(root)
        w = DupWalker("dump.i")
        root.walk(w, hit.NodeType.All)
        if w.errors:
            print("\n".join(w.errors))
        self.assertEqual(len(w.errors), 0)
        return root

    def getBlockSections(self, node):
        return {c.path(): c for c in node.children(node_type=hit.NodeType.Section)}

    def getBlockParams(self, node):
        return {c.path(): c for c in node.children(node_type=hit.NodeType.Field)}

    def testInputFileFormat(self):
        """
        Some basic checks to see if some data
        is there and is in the right location.
        """
        root = self.getInputFileFormat()
        root_sections = self.getBlockSections(root)
        self.assertIn("Executioner", root_sections)
        self.assertIn("BCs", root_sections)
        bcs_sections = self.getBlockSections(root_sections["BCs"])
        self.assertIn("Periodic", bcs_sections)
        self.assertIn("*", bcs_sections)
        star = bcs_sections["*"]
        bcs_star_params = self.getBlockParams(star)
        bcs_star_sections = self.getBlockSections(star)
        self.assertIn("active", bcs_star_params)
        self.assertIn("<types>", bcs_star_sections)
        bcs_star_types_sections = self.getBlockSections(bcs_star_sections["<types>"])
        self.assertIn("<DirichletBC>", bcs_star_types_sections)
        periodic_children = self.getBlockSections(bcs_sections["Periodic"])
        self.assertEqual(len(periodic_children.keys()), 1)
        self.assertIn("*", periodic_children)

        self.assertNotIn("<types>", bcs_sections)

        exe_sections = self.getBlockSections(root_sections["Executioner"])
        self.assertIn("<types>", exe_sections)
        exe_types_sections = self.getBlockSections(exe_sections["<types>"])
        self.assertIn("<Transient>", exe_types_sections)

        # Preconditioning has a Preconditioning/*/* syntax which is unusual
        self.assertIn("Preconditioning", root_sections)
        p = root_sections["Preconditioning"]
        pc_sections = self.getBlockSections(p)
        pc_star_sections = self.getBlockSections(pc_sections["*"])
        pc_star_star_sections = self.getBlockSections(pc_star_sections["*"])
        pc_star_star_types_sections = self.getBlockSections(pc_star_star_sections["<types>"])
        split_params = self.getBlockParams(pc_star_star_types_sections["<Split>"])
        self.assertIn("splitting_type", split_params)
        self.assertIn("petsc_options", split_params)

        # Make sure the default dump has test objects
        self.assertIn("ApplyInputParametersTest", root_sections)

    def testInputFileFormatSearch(self):
        """
        Make sure parameter search works
        """
        root = self.getInputFileFormat(["initial_steps"])
        section_map = {c.path(): c for c in root.children(node_type=hit.NodeType.Section)}
        self.assertNotIn("Executioner", section_map)
        self.assertNotIn("BCs", section_map)
        self.assertIn("Adaptivity", section_map)
        self.assertEqual(len(section_map.keys()), 1)
        adaptivity = section_map["Adaptivity"]
        params = {c.path(): c for c in adaptivity.children(node_type=hit.NodeType.Field)}
        self.assertIn("initial_steps", params)
        self.assertEqual(len(params.keys()), 1)

    def testLineInfo(self):
        """
        Make sure file/line information works
        """
        all_data = self.getJsonData()
        data = all_data["blocks"]
        adapt = data["Adaptivity"]["actions"]["SetAdaptivityOptionsAction"]
        fi = adapt["file_info"]
        self.assertEqual(len(fi.keys()), 1)
        fname = fi.keys()[0]
        # Clang seems to have the full path name for __FILE__
        # gcc seems to just use the path that is given on the command line, which won't include "framework"
        self.assertTrue(fname.endswith(os.path.join("src", "base", "Moose.C")), 'file "{}" found instead'.format(fname))
        self.assertGreater(fi[fname], 0)

        fi = adapt["tasks"]["set_adaptivity_options"]["file_info"]
        self.assertEqual(len(fi.keys()), 1)
        fname = fi.keys()[0]
        self.assertTrue(fname.endswith(os.path.join("src", "actions", "SetAdaptivityOptionsAction.C")))
        self.assertGreater(fi[fname], 0)

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
