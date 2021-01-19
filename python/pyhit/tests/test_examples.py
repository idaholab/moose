#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import unittest
import pyhit

class TestExamples(unittest.TestCase):
    def test(self):

        # MOOSEDOCS:example-begin
        # Load the packages
        import pyhit
        import moosetree

        # Read the file
        root = pyhit.load('input.i')

        # Locate and modify "x_max" parameter for the mesh
        mesh = moosetree.find(root, func=lambda n: n.fullpath == '/Mesh/gen')
        mesh["x_max"] = 4

        # Set the comment on altered parameter
        mesh.setComment("x_max", "Changed from 3 to 4")

        # Write the modified file
        pyhit.write("input_modified.i", root)

        # MOOSEDOCS:example-end
        self.assertEqual(mesh["x_max"], 4)
        self.assertEqual(mesh.comment("x_max"), "Changed from 3 to 4")

        out = mesh.render()
        self.assertIn("x_max = 4", out)
        self.assertIn("Changed from 3 to 4", out)

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
