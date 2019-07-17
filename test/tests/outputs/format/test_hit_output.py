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

class TestHITBase(unittest.TestCase):
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
        root = hit.parse("dump.i", output)
        hit.explode(root)
        w = DupWalker("dump.i")
        root.walk(w, hit.NodeType.All)
        if w.errors:
            print("\n".join(w.errors))
        self.assertEqual(len(w.errors), 0)
        return root


class TestInputFileFormat(TestHITBase):
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


class TestInputFileFormatSearch(TestHITBase):
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

if __name__ == '__main__':
    unittest.main(__name__, verbosity=2)
