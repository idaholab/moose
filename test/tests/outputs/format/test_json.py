#!/usr/bin/env python
import os, sys
import subprocess
import json
import unittest
from FactorySystem.ParseGetPot import readInputFile

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

    def testJson(self):
        """
        Some basic checks to see if some data
        is there and is in the right location.
        """
        data = self.getJsonData()
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

    def testJsonSearch(self):
        """
        Make sure parameter search works
        """
        data = self.getJsonData(["initial_marker"])
        self.assertNotIn("Executioner", data)
        self.assertNotIn("BCs", data)
        self.assertIn("Adaptivity", data)
        self.assertEqual(len(data.keys()), 1)
        params = data["Adaptivity"]["actions"]["SetAdaptivityOptionsAction"]["parameters"]
        self.assertIn("initial_marker", params)
        self.assertEqual(len(params.keys()), 1)

        # test to make sure it matches blocks as well
        data = self.getJsonData(["diffusion"])
        self.assertEqual(len(data.keys()), 2)
        self.assertIn("BadKernels", data)
        self.assertIn("Kernels", data)
        diff = data["Kernels"]["star"]["subblock_types"]["Diffusion"]
        self.assertIn("eigen_kernel", diff["parameters"])

    def getInputFileFormat(self, extra=[]):
        """
        Does a dump and uses the GetPotParser to parse the output.
        """
        args = ["--dump"] + extra
        output = run_app(args)
        self.assertNotEqual(len(output), 0)
        with open("dump.i", "w") as f:
            f.write(output)
        return readInputFile("dump.i")

    def testInputFileFormat(self):
        """
        Some basic checks to see if some data
        is there and is in the right location.
        """
        root = self.getInputFileFormat()
        self.assertIn("Executioner", root.children_list)
        self.assertIn("BCs", root.children_list)
        bcs = root.children["BCs"]
        self.assertIn("Periodic", bcs.children_list)
        self.assertIn("*", bcs.children_list)
        star = bcs.children["*"]
        self.assertIn("active", star.params_list)
        self.assertIn("<DirichletBC>", star.children["<types>"].children_list)
        self.assertEqual(bcs.children["Periodic"].children_list, ["*"])

        self.assertNotIn("<types>", bcs.children_list)

        exe = root.children["Executioner"]
        self.assertIn("<types>", exe.children_list)
        self.assertIn("<Transient>", exe.children["<types>"].children_list)

        # Preconditioning has a Preconditioning/*/* syntax which is unusual
        self.assertIn("Preconditioning", root.children_list)
        p = root.children["Preconditioning"]
        split = p.children["*"].children["*"].children["<types>"].children["<Split>"]
        self.assertIn("splitting_type", split.params_list)
        # There is a problem with the GetPotParser parsing this because
        # the comments got split and there is a unmatched " on a line of the comment.
        # This breaks the parser and it doesn't read in all the parameters.
        # self.assertIn("petsc_options", split.params_list)

    def testInputFileFormatSearch(self):
        """
        Make sure parameter search works
        """
        root = self.getInputFileFormat(["initial_steps"])
        self.assertNotIn("Executioner", root.children_list)
        self.assertNotIn("BCs", root.children_list)
        self.assertIn("Adaptivity", root.children_list)
        self.assertEqual(len(root.children_list), 1)
        self.assertIn("initial_steps", root.children["Adaptivity"].params_list)
        self.assertEqual(len(root.children["Adaptivity"].params_list), 1)

    def testLineInfo(self):
        """
        Make sure file/line information works
        """
        data = self.getJsonData()
        adapt = data["Adaptivity"]["actions"]["SetAdaptivityOptionsAction"]
        fi = adapt["file_info"]
        self.assertEqual(len(fi.keys()), 1)
        fname = fi.keys()[0]
        # Clang seems to have the full path name for __FILE__
        # gcc seems to just use the path that is given on the command line, which won't include "framework"
        self.assertTrue(fname.endswith(os.path.join("src", "parser", "MooseSyntax.C")))
        self.assertGreater(fi[fname], 0)

        fi = adapt["tasks"]["set_adaptivity_options"]["file_info"]
        self.assertEqual(len(fi.keys()), 1)
        fname = fi.keys()[0]
        self.assertTrue(fname.endswith(os.path.join("src", "base", "Moose.C")))
        self.assertGreater(fi[fname], 0)

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
