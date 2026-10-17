# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Unit tests for how Runner reads and reports output that is not valid UTF-8."""

import io
import unittest
from unittest import mock

from TestHarness.runners.Runner import Runner


class TestDescribeInvalidUTF8(unittest.TestCase):
    """Tests Runner.describeInvalidUTF8, which locates invalid bytes in a stream."""

    def testEachSequenceIsReported(self):
        """Every invalid byte is reported, with its position in the whole stream."""
        data = b"before\xff\xfeafter\n"
        out = Runner.describeInvalidUTF8(data, "stdout")

        self.assertIn("Begin invalid UTF-8 in stdout", out)
        self.assertIn("End invalid UTF-8 in stdout", out)
        # Offsets are relative to the whole stream, not to the remainder
        # that is still being decoded
        self.assertIn(f"offset 6 of {len(data)}: invalid start byte: b'\\xff'", out)
        self.assertIn(f"offset 7 of {len(data)}: invalid start byte: b'\\xfe'", out)
        self.assertEqual(out.count("offset "), 2)

    def testSurroundingBytes(self):
        """The bytes on either side of an invalid byte are shown for context."""
        out = Runner.describeInvalidUTF8(b"before\xffafter\n", "stdout")
        self.assertIn("before: b'before'", out)
        self.assertIn("after:  b'after\\n'", out)

    def testContextIsBounded(self):
        """Only the requested number of surrounding bytes is shown."""
        data = b"a" * 100 + b"\xff" + b"b" * 100
        out = Runner.describeInvalidUTF8(data, "stderr", context=10)

        self.assertIn(f"before: {(b'a' * 10)!r}", out)
        self.assertIn(f"after:  {(b'b' * 10)!r}", out)

    def testContextIsClippedAtTheStartOfTheData(self):
        """Context does not run off the start of the data."""
        out = Runner.describeInvalidUTF8(b"ab\xffcd", "stdout", context=10)
        self.assertIn("offset 2 of 5", out)
        self.assertIn("before: b'ab'", out)
        self.assertIn("after:  b'cd'", out)

    def testTruncatedSequenceAtTheEnd(self):
        """A multi-byte character cut off at the end of the data is reported."""
        # The first two bytes of a three byte sequence
        out = Runner.describeInvalidUTF8(b"ab\xe2\x82", "stdout")
        self.assertIn("offset 2 of 4: unexpected end of data: b'\\xe2\\x82'", out)
        self.assertEqual(out.count("offset "), 1)


class TestReadOutput(unittest.TestCase):
    """Tests Runner.readOutput, which decodes a stream and flags invalid bytes."""

    def setUp(self):
        self.runner = object.__new__(Runner)
        self.runner.job = mock.Mock()

    def read(self, data, name="output"):
        return self.runner.readOutput(io.BytesIO(data), name)

    def testValidOutputIsUnchanged(self):
        """Valid UTF-8 output is returned as is with no caveat."""
        self.assertEqual(self.read("caf\u00e9\n".encode("utf-8")), "caf\u00e9\n")
        self.runner.job.addCaveats.assert_not_called()
        self.runner.job.setStatus.assert_not_called()

    def testNewlineIsAppended(self):
        """Output that does not end in a newline gets one."""
        self.assertEqual(self.read(b"no newline"), "no newline\n")

    def testEmptyOutput(self):
        """Empty output stays empty and is not flagged."""
        self.assertEqual(self.read(b""), "")
        self.runner.job.addCaveats.assert_not_called()

    def testInvalidOutputIsKeptAndFlagged(self):
        """Invalid bytes are kept as escapes and described, and the test gets a
        caveat instead of failing."""
        out = self.read(b"before\xff\xfeafter\n", "stdout")

        # The bytes stay in the output as escapes
        self.assertTrue(out.startswith("before\\xff\\xfeafter\n"))
        # Each invalid sequence is described
        self.assertIn("Begin invalid UTF-8 in stdout", out)
        self.assertIn("offset 6 of 14: invalid start byte: b'\\xff'", out)
        self.assertIn("offset 7 of 14: invalid start byte: b'\\xfe'", out)
        # The caveat names the stream, and the test is not failed
        self.runner.job.addCaveats.assert_called_once_with("invalid UTF-8 in stdout")
        self.runner.job.setStatus.assert_not_called()

    def testInvalidOutputWithoutTrailingNewline(self):
        """The description starts on its own line when there is no trailing newline."""
        out = self.read(b"before\xffafter", "stderr")
        self.assertTrue(out.startswith("before\\xffafter\n"))
        self.assertIn("Begin invalid UTF-8 in stderr", out)

    def testReadFailureSetsError(self):
        """A stream that cannot be read puts the job in an error state."""
        stream = mock.Mock()
        stream.seek.side_effect = OSError("boom")

        self.assertEqual(self.runner.readOutput(stream), "")
        self.runner.job.setStatus.assert_called_once_with(
            self.runner.job.error, "error reading output"
        )


if __name__ == "__main__":
    unittest.main()
