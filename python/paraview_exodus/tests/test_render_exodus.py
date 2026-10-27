#!/usr/bin/env python3
# * This file is part of the MOOSE framework
# * https://mooseframework.inl.gov
# *
# * All rights reserved, see COPYRIGHT for full restrictions
# * https://github.com/idaholab/moose/blob/master/COPYRIGHT
# *
# * Licensed under LGPL 2.1, please see LICENSE for details
# * https://www.gnu.org/licenses/lgpl-2.1.html

import os
import shutil
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))

# render_exodus imports paraview.simple only when it is available, so the pure
# helpers exercised here are importable under a plain python interpreter.
import render_exodus

SCRIPT_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
WRAPPER = os.path.join(SCRIPT_DIR, "visualize_exodus.sh")


class TestParseColor(unittest.TestCase):
    def testNamed(self):
        self.assertEqual(render_exodus.parse_color("white"), (1.0, 1.0, 1.0))
        self.assertEqual(render_exodus.parse_color(" BLACK "), (0.0, 0.0, 0.0))
        self.assertIsNone(render_exodus.parse_color("transparent"))

    def testHex(self):
        self.assertEqual(render_exodus.parse_color("#ffffff"), (1.0, 1.0, 1.0))
        self.assertEqual(render_exodus.parse_color("#000"), (0.0, 0.0, 0.0))
        r, g, b = render_exodus.parse_color("#804020")
        self.assertAlmostEqual(r, 128 / 255.0)
        self.assertAlmostEqual(g, 64 / 255.0)
        self.assertAlmostEqual(b, 32 / 255.0)

    def testRGB(self):
        self.assertEqual(render_exodus.parse_color("0,0.5,1"), (0.0, 0.5, 1.0))
        # A component above 1 means the triple is on 0-255.
        self.assertEqual(render_exodus.parse_color("255,0,0"), (1.0, 0.0, 0.0))

    def testInvalid(self):
        # parse_color tries three forms in order: a name from NAMED_COLORS, a
        # '#rrggbb' hex string, then a comma separated triple. A word that is
        # none of those reaches the end of the function with nothing to parse.
        # Any color name outside NAMED_COLORS lands here, valid elsewhere or not.
        with self.assertRaisesRegex(ValueError, "Unrecognized color"):
            render_exodus.parse_color("foo")

        # The comma puts this on the triple path, which needs all three of r, g,
        # and b, so a pair is reported as the wrong number of components rather
        # than as an unrecognized color.
        with self.assertRaisesRegex(ValueError, "three components"):
            render_exodus.parse_color("1,2")


class TestContrastingColor(unittest.TestCase):
    def testContrast(self):
        white = (1.0, 1.0, 1.0)
        black = (0.0, 0.0, 0.0)
        self.assertEqual(render_exodus.contrasting_color(white), black)
        self.assertEqual(render_exodus.contrasting_color(black), white)
        full_blue = (0.0, 0.0, 1.0)
        full_green = (0.0, 1.0, 0.0)
        self.assertEqual(render_exodus.contrasting_color(full_blue), white)
        self.assertEqual(render_exodus.contrasting_color(full_green), black)


class TestIsIdArray(unittest.TestCase):
    def testReaderArrays(self):
        for name in (
            "ids",
            "object_id",
            "ObjectId",
            "GlobalNodeId",
            "GlobalElementId",
            "PedigreeElementId",
            "file_id",
            "element_side",
        ):
            self.assertTrue(render_exodus.is_id_array(name), name)

    def testSolutionFields(self):
        # Field names that merely end in the letters "id" are solution fields.
        for name in ("temperature", "disp_x", "u", "fluid", "solid", "rhoid"):
            self.assertFalse(render_exodus.is_id_array(name), name)


class TestFitResolution(unittest.TestCase):
    def testWideGeometryUsesWidth(self):
        # An aspect of 2.0 is wider than the 800x600 box, so width is the
        # limiting dimension: the frame is 800 wide and 800 / 2.0 tall.
        self.assertEqual(
            render_exodus.fit_resolution(800, 600, 2.0, False), (800, 400, 0)
        )

    def testTallGeometryUsesHeight(self):
        # An aspect of 0.5 is taller than the box, so height limits instead: the
        # frame is 600 tall and 600 * 0.5 wide.
        self.assertEqual(
            render_exodus.fit_resolution(800, 600, 0.5, False), (300, 600, 0)
        )

    def testEvenDimensions(self):
        # yuv420p requires even dimensions in both directions.
        for aspect in (0.37, 1.0, 3.1, 7.9):
            w, h, _ = render_exodus.fit_resolution(801, 601, aspect, False)
            self.assertEqual(w % 2, 0)
            self.assertEqual(h % 2, 0)

    def testScalarBarReserve(self):
        plain_w, _, _ = render_exodus.fit_resolution(800, 600, 2.0, False)
        bar_w, _, reserve = render_exodus.fit_resolution(800, 600, 2.0, True)
        self.assertGreaterEqual(reserve, render_exodus.SCALAR_BAR_RESERVE_PX)
        self.assertGreater(bar_w, plain_w)


class TestWrapper(unittest.TestCase):
    """Cover visualize_exodus.sh's status line handling with stub pvpython and
    ffmpeg executables, so no ParaView install or real encode is needed."""

    def setUp(self):
        self.tmp = tempfile.mkdtemp()
        self.bin = os.path.join(self.tmp, "bin")
        os.mkdir(self.bin)

    def tearDown(self):
        shutil.rmtree(self.tmp, ignore_errors=True)

    def stub(self, name, body):
        path = os.path.join(self.bin, name)
        with open(path, "w") as handle:
            handle.write("#!/usr/bin/env bash\n" + body)
        os.chmod(path, 0o755)

    def wrapper(self, args):
        env = dict(os.environ)
        env["PATH"] = self.bin + os.pathsep + env["PATH"]
        return subprocess.run(
            ["bash", WRAPPER] + args,
            cwd=self.tmp,
            env=env,
            capture_output=True,
            text=True,
        )

    def echoArgs(self, tag):
        return (
            'printf "%s"; for a in "$@"; do printf " [%%s]" "$a"; done; printf "\\n"\n'
            % tag
        )

    def testAnimationPrefixWithSpaces(self):
        # The prefix is recovered from a tab-delimited status line, so a name
        # containing spaces reaches ffmpeg as a single argument.
        prefix = "beam result"
        self.stub(
            "pvpython",
            "printf 'ANIMATION\\t%s\\tframes=3\\tresolution=802x600\\n' 'beam result'\n",
        )
        self.stub("ffmpeg", self.echoArgs("FFMPEG"))
        frames = [os.path.join(self.tmp, "%s.%04d.png" % (prefix, i)) for i in range(3)]
        for frame in frames:
            open(frame, "w").close()

        result = self.wrapper(["beam result.e", "--output", prefix])

        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("[%s.%%04d.png]" % prefix, result.stdout)
        self.assertIn("[%s.mp4]" % prefix, result.stdout)
        self.assertIn("802x600", result.stderr)
        for frame in frames:
            self.assertFalse(os.path.exists(frame))

    def testKeepFrames(self):
        prefix = "beam result"
        self.stub(
            "pvpython",
            "printf 'ANIMATION\\t%s\\tframes=1\\tresolution=800x600\\n' 'beam result'\n",
        )
        self.stub("ffmpeg", self.echoArgs("FFMPEG"))
        frame = os.path.join(self.tmp, "%s.0000.png" % prefix)
        open(frame, "w").close()

        result = self.wrapper(["beam result.e", "--output", prefix, "--keep-frames"])

        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertTrue(os.path.exists(frame))

    def testFfmpegPassthrough(self):
        self.stub(
            "pvpython",
            "printf 'ANIMATION\\tout\\tframes=1\\tresolution=800x600\\n'\n",
        )
        self.stub("ffmpeg", self.echoArgs("FFMPEG"))

        result = self.wrapper(["out.e", "--", "-crf", "18"])

        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("[-crf] [18]", result.stdout)

    def testStillReportsFullPath(self):
        self.stub("pvpython", "printf 'STILL\\tbeam result.png\\n'\n")

        result = self.wrapper(["beam result.e"])

        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("beam result.png", result.stderr)

    def testNoStatusLine(self):
        # --list-fields prints no status line; the wrapper exits cleanly.
        self.stub("pvpython", "printf 'Point (nodal) fields:\\n  temperature\\n'\n")

        result = self.wrapper(["out.e", "--list-fields"])

        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("temperature", result.stdout)


if __name__ == "__main__":
    unittest.main(verbosity=2)
