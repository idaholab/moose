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
import sys
import io
import unittest
from unittest.mock import patch, MagicMock, mock_open
import subprocess
import tempfile

# Add the scripts directory to the path so we can import pre_check
sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
import pre_check

# Environment that disables every check — used as a base for focused tests.
ALL_CHECKS_OFF = {
    "CHECK_TICKET_REFERENCE": "0",
    "CHECK_KEYWORDS": "0",
    "CHECK_STYLE": "0",
    "CHECK_EOF": "0",
    "CHECK_EXECUTABLES": "0",
    "CHECK_WHITESPACE": "0",
    "CHECK_TABS": "0",
    "CHECK_CLASSIFIED": "0",
    "CHECK_UNICODE": "0",
    "CHECK_INCLUDE_GUARDS": "0",
    "CHECK_WINDOWS_FILES": "0",
    "CHECK_BANNED_FUNCS": "0",
}


def make_precheck(file_entries):
    """Create a PreCheck instance with cached file entries for method tests."""
    check = pre_check.PreCheck.__new__(pre_check.PreCheck)
    check.file_entries = file_entries
    check.check_cpp_whitespace = False
    check.check_f_whitespace = False
    return check


class TestHelpers(unittest.TestCase):
    """Test helper functions"""

    @patch("pre_check.subprocess.run")
    def test_changed_files(self, mock_run):
        """Test changed_files() returns paths touched by the requested range"""
        mock_run.return_value = subprocess.CompletedProcess(
            args=["git", "diff"],
            returncode=0,
            stdout="file1.py\x00dir/file2.C\x00",
        )
        files = pre_check.changed_files("base", "HEAD")
        self.assertEqual(files, {"file1.py", "dir/file2.C"})
        mock_run.assert_called_once_with(
            [
                "git",
                "diff",
                "--name-only",
                "-z",
                "--diff-filter=ACMRT",
                "base..HEAD",
            ],
            capture_output=True,
            text=True,
            check=True,
        )

    @patch("pre_check.subprocess.run")
    def test_git_files(self, mock_run):
        """Test git_files() function"""
        mock_run.return_value = subprocess.CompletedProcess(
            args=["git", "ls-files"],
            returncode=0,
            stdout=(
                "100644 abc123 0\tfile1.py\x00"
                "100755 abc123 0\tfile2.C\x00"
                "100644 abc123 0\tcontrib/skip.py\x00"
                "100644 abc123 0\tdir/contrib/skip.C\x00"
                "100644 abc123 0\tfile3.h\x00"
                "120000 abc123 0\tlink.py\x00"
                "160000 abc123 0\tsubmodule\x00"
            ),
        )
        files = pre_check.git_files("*.py", "*.[Ch]")
        self.assertIn("file1.py", files)
        self.assertIn("file2.C", files)
        self.assertIn("file3.h", files)
        self.assertNotIn("contrib/skip.py", files)
        self.assertNotIn("dir/contrib/skip.C", files)
        self.assertNotIn("link.py", files)
        self.assertNotIn("submodule", files)
        mock_run.assert_called_once_with(
            [
                "git",
                "ls-files",
                "-z",
                "--stage",
                "--cached",
                "--",
                "*.py",
                "*.[Ch]",
            ],
            capture_output=True,
            text=True,
            check=True,
        )

    @patch("pre_check.subprocess.run")
    def test_git_files_filters_touched_files(self, mock_run):
        """Test git_files() only returns requested touched files"""
        mock_run.return_value = subprocess.CompletedProcess(
            args=["git", "ls-files"],
            returncode=0,
            stdout=(
                "100644 abc123 0\tfile1.py\x00"
                "100644 abc123 0\tfile2.C\x00"
                "120000 abc123 0\tlink.py\x00"
            ),
        )
        files = pre_check.git_files(
            "*.py", "*.[Ch]", touched_files={"file2.C", "link.py"}
        )
        self.assertEqual(files, ["file2.C"])

    @patch("pre_check.subprocess.run")
    def test_precheck_files_uses_cached_entries(self, mock_run):
        """Test PreCheck.files() filters cached entries without calling git"""
        check = make_precheck(
            [
                ("100644", "file1.py"),
                ("100644", "file2.C"),
            ]
        )
        files = check.files("*.py")
        self.assertEqual(files, ["file1.py"])
        mock_run.assert_not_called()

    def test_read_text(self):
        """Test read_text() function"""
        with patch("builtins.open", mock_open(read_data="test content")):
            result = pre_check.read_text("dummy.txt")
            self.assertEqual(result, "test content")


class TestTicketReferences(unittest.TestCase):
    """Test ticket reference detection"""

    @patch("pre_check.subprocess.run")
    def test_ticket_references_found(self, mock_run):
        """Test detection of ticket references in commit messages"""
        mock_run.return_value = MagicMock(
            stdout="Commit 1\nFix issue #1234\nCommit 2\nmoose/issues/5678\nCommit 3\nmoose/pull/9012"
        )
        result = pre_check.ticket_references("HEAD~3", "HEAD")
        self.assertIn("#1234", result)
        self.assertIn("moose/issues/5678", result)
        self.assertIn("moose/pull/9012", result)

    @patch("pre_check.subprocess.run")
    def test_ticket_references_not_found(self, mock_run):
        """Test when no ticket references are found"""
        mock_run.return_value = MagicMock(
            stdout="Commit 1\nNo ticket here\nCommit 2\nAnother commit"
        )
        result = pre_check.ticket_references("HEAD~2", "HEAD")
        self.assertEqual(result, "")


class TestFileSetHelpers(unittest.TestCase):
    """Test file set helper functions"""

    def test_files_for_tabs_except_input(self):
        """Test files_for_tabs_except_input()"""
        check = make_precheck(
            [("100644", "file1.C"), ("100644", "file2.i"), ("100644", "file3.py")]
        )
        result = check.files_for_tabs_except_input()
        self.assertEqual(result, ["file1.C", "file3.py"])

    def test_files_for_tabs(self):
        """Test files_for_tabs()"""
        check = make_precheck(
            [("100644", "file1.C"), ("100644", "file2.i"), ("100644", "file3.py")]
        )
        result = check.files_for_tabs()
        self.assertEqual(result, ["file1.C", "file2.i", "file3.py"])

    def test_files_for_whitespace_cpp_enabled(self):
        """Test files_for_whitespace() with C++ files"""
        check = make_precheck(
            [("100644", "file1.C"), ("100644", "file2.py"), ("100644", "file3.i")]
        )
        check.check_cpp_whitespace = True
        result = check.files_for_whitespace()
        self.assertEqual(result, ["file1.C", "file2.py", "file3.i"])

    def test_files_for_whitespace_fortran_enabled(self):
        """Test files_for_whitespace() with Fortran files"""
        check = make_precheck(
            [
                ("100644", "file1.i"),
                ("100644", "file2.py"),
                ("100644", "file3.F"),
                ("100644", "file4.f90"),
                ("100644", "file5.C"),
            ]
        )
        check.check_f_whitespace = True
        result = check.files_for_whitespace()
        self.assertEqual(result, ["file1.i", "file2.py", "file3.F", "file4.f90"])


class TestIndividualChecks(unittest.TestCase):
    """Test individual check functions"""

    @patch("pre_check.read_text")
    def test_find_tabs(self, mock_read_text):
        """Test find_tabs() function"""
        check = make_precheck([])
        mock_read_text.side_effect = lambda f: (
            "line with\ttab" if f == "file1.C" else "no tabs"
        )
        result = check.find_tabs(["file1.C", "file2.C"])
        self.assertEqual(result, ["file1.C"])

    @patch("pre_check.read_text")
    def test_banned_keywords_cout(self, mock_read_text):
        """Test banned_keywords() detects std::cout"""
        check = make_precheck([("100644", "file1.C")])
        mock_read_text.return_value = 'std::cout << "hello"'
        result = check.banned_keywords()
        self.assertEqual(result, ["file1.C"])

    @patch("pre_check.read_text")
    def test_banned_keywords_printf(self, mock_read_text):
        """Test banned_keywords() detects printf"""
        check = make_precheck([("100644", "file1.C")])
        mock_read_text.return_value = 'printf("test")'
        result = check.banned_keywords()
        self.assertEqual(result, ["file1.C"])

    @patch("pre_check.read_text")
    def test_banned_keywords_sleep_exception(self, mock_read_text):
        """Test banned_keywords() allows sleep in SlowProblem.C"""
        check = make_precheck([("100644", "SlowProblem.C"), ("100644", "other.C")])
        mock_read_text.side_effect = lambda f: (
            "sleep(1)" if f in ["SlowProblem.C", "other.C"] else ""
        )
        result = check.banned_keywords()
        self.assertEqual(result, ["other.C"])

    @patch("pre_check.read_text")
    def test_banned_funcs(self, mock_read_text):
        """Test banned_funcs() detects deprecated MOOSE functions"""
        check = make_precheck([("100644", "file1.C")])
        mock_read_text.return_value = 'mooseError2("test")'
        result = check.banned_funcs()
        self.assertEqual(result, ["file1.C"])

    @patch("pre_check.read_text")
    def test_classified_keywords(self, mock_read_text):
        """Test classified_keywords() detects proprietary/classified"""
        check = make_precheck([("100644", "file1.C"), ("100644", "pre_check.py")])
        mock_read_text.side_effect = lambda f: (
            "p r o p r i e t a r y" if f == "file1.C" else "proprietary"
        )
        result = check.classified_keywords()
        self.assertEqual(result, ["file1.C"])

    @patch("builtins.open", new_callable=mock_open, read_data="line1  \nline2\n")
    def test_trailing_whitespace_files(self, mock_file):
        """Test trailing_whitespace_files() detects trailing spaces"""
        check = make_precheck([])
        result = check.trailing_whitespace_files(["file1.py"])
        self.assertEqual(result, ["file1.py"])

    @patch("builtins.open", new_callable=mock_open, read_data="line1\nline2\n")
    def test_trailing_whitespace_files_no_whitespace(self, mock_file):
        """Test trailing_whitespace_files() with no trailing spaces"""
        check = make_precheck([])
        result = check.trailing_whitespace_files(["file1.py"])
        self.assertEqual(result, [])

    def test_no_newline_at_eof_files(self):
        """Test no_newline_at_eof_files() detects missing newline"""
        check = make_precheck([("100644", "file1.py"), ("100644", "file2.py")])

        with patch("builtins.open", mock_open(read_data=b"content")):
            result = check.no_newline_at_eof_files()
            self.assertEqual(result, ["file1.py", "file2.py"])

    @patch("os.stat")
    @patch("os.access")
    def test_find_bad_executables(self, mock_access, mock_stat):
        """Test find_bad_executables() detects incorrectly executable files"""
        check = make_precheck(
            [
                ("100755", "file1.txt"),
                ("100755", "script.py"),
                ("100644", "README.md"),
            ]
        )

        result = check.find_bad_executables()
        self.assertIn("file1.txt", result)
        self.assertNotIn("script.py", result)
        self.assertNotIn("README.md", result)
        mock_stat.assert_not_called()
        mock_access.assert_not_called()

    @patch("pre_check.read_text")
    def test_style_files_bad(self, mock_read_text):
        """Test style_files() detects control keywords without a space"""
        check = make_precheck([("100644", "file1.C")])
        mock_read_text.return_value = "if(x) { return 1; }"
        result = check.style_files()
        self.assertEqual(result, ["file1.C"])

    @patch("pre_check.read_text")
    def test_style_files_ok(self, mock_read_text):
        """Test style_files() passes when keywords have proper spacing"""
        check = make_precheck([("100644", "file1.C")])
        mock_read_text.return_value = "if (x) { return 1; }"
        result = check.style_files()
        self.assertEqual(result, [])

    @patch("pre_check.read_text")
    def test_include_guard_files(self, mock_read_text):
        """Test include_guard_files() detects old-style guards"""
        check = make_precheck([("100644", "header.h")])
        mock_read_text.return_value = (
            "#ifndef HEADER_H\n#define HEADER_H\ncontent\n#endif"
        )
        result = check.include_guard_files()
        self.assertEqual(result, ["header.h"])

    def test_windows_line_endings(self):
        """Test windows_line_endings() detects CRLF"""
        check = make_precheck([("100644", "file1.C")])
        with patch("builtins.open", mock_open(read_data=b"line1\r\nline2\r\n")):
            result = check.windows_line_endings()
        self.assertEqual(result, ["file1.C"])


class TestUnicodeChecks(unittest.TestCase):
    """Test Unicode character checking"""

    def test_is_allowed_manual_ascii(self):
        """Test _is_allowed_manual() with ASCII characters"""
        self.assertTrue(pre_check._is_allowed_manual("a"))
        self.assertTrue(pre_check._is_allowed_manual("Z"))
        self.assertTrue(pre_check._is_allowed_manual("0"))

    def test_is_allowed_manual_latin_extended(self):
        """Test _is_allowed_manual() with Latin Extended characters"""
        self.assertTrue(pre_check._is_allowed_manual("é"))  # Latin-1
        self.assertTrue(pre_check._is_allowed_manual("ł"))  # Latin Extended-A
        self.assertTrue(pre_check._is_allowed_manual("ș"))  # Latin Extended-A

    def test_is_allowed_manual_greek(self):
        """Test _is_allowed_manual() with Greek characters"""
        self.assertTrue(pre_check._is_allowed_manual("α"))
        self.assertTrue(pre_check._is_allowed_manual("Ω"))

    def test_is_allowed_manual_math_symbols(self):
        """Test _is_allowed_manual() with math symbols"""
        self.assertTrue(pre_check._is_allowed_manual("∇"))  # Nabla
        self.assertTrue(pre_check._is_allowed_manual("∫"))  # Integral
        self.assertTrue(pre_check._is_allowed_manual("±"))  # Plus-minus
        self.assertTrue(pre_check._is_allowed_manual("∂"))  # Partial

    def test_is_allowed_manual_superscripts(self):
        """Test _is_allowed_manual() with superscripts/subscripts"""
        self.assertTrue(pre_check._is_allowed_manual("²"))
        self.assertTrue(pre_check._is_allowed_manual("₃"))

    def test_is_allowed_manual_disallowed(self):
        """Test _is_allowed_manual() with disallowed characters"""
        self.assertFalse(pre_check._is_allowed_manual("😀"))  # Emoji
        self.assertFalse(pre_check._is_allowed_manual("中"))  # CJK

    def test_disallowed_spans(self):
        """Test _disallowed_spans() function"""
        text = "hello 😀 world"
        spans = pre_check._disallowed_spans(text)
        self.assertEqual(len(spans), 1)
        self.assertEqual(spans[0][0], 6)  # Position of emoji
        self.assertEqual(spans[0][1], "😀")

    def test_get_line_col(self):
        """Test _get_line_col() function"""
        text = "line1\nline2\nline3"
        line, col = pre_check._get_line_col(text, 0)
        self.assertEqual((line, col), (1, 1))

        line, col = pre_check._get_line_col(text, 6)
        self.assertEqual((line, col), (2, 1))

    @patch("pre_check.read_text")
    def test_unicode_files(self, mock_read_text):
        """Test unicode_files() detects disallowed Unicode"""
        check = make_precheck([("100644", "file1.C")])
        mock_read_text.return_value = "hello 😀 world"
        bad_files, locations = check.unicode_files()
        self.assertEqual(bad_files, ["file1.C"])
        self.assertEqual(len(locations), 1)
        self.assertIn("file1.C:1:7", locations[0])


class TestPrecheckErrors(unittest.TestCase):
    """Test main precheck_errors() function"""

    @patch.dict(os.environ, ALL_CHECKS_OFF)
    @patch("pre_check.changed_files")
    def test_precheck_errors_all_disabled(self, mock_changed_files):
        """Test precheck_errors() with all checks disabled"""
        result = pre_check.precheck_errors("HEAD~1", "HEAD")
        self.assertEqual(result, 0)
        mock_changed_files.assert_not_called()

    @patch.dict(os.environ, {**ALL_CHECKS_OFF, "CHECK_TICKET_REFERENCE": "1"})
    @patch("pre_check.ticket_references")
    @patch("pre_check.changed_files")
    @patch("sys.stdout")
    def test_precheck_errors_ticket_pass(
        self, mock_stdout, mock_changed_files, mock_ticket
    ):
        """Test precheck_errors() with valid ticket reference"""
        mock_ticket.return_value = "Fix #1234"
        result = pre_check.precheck_errors("HEAD~1", "HEAD")
        self.assertEqual(result, 0)
        mock_changed_files.assert_not_called()

    @patch.dict(os.environ, {**ALL_CHECKS_OFF, "CHECK_TICKET_REFERENCE": "1"})
    @patch("pre_check.ticket_references")
    @patch("pre_check.changed_files")
    @patch("pre_check.subprocess.run")
    @patch("sys.stdout")
    def test_precheck_errors_ticket_fail(
        self, mock_stdout, mock_run, mock_changed_files, mock_ticket
    ):
        """Test precheck_errors() without ticket reference"""
        mock_ticket.return_value = ""
        mock_run.return_value = MagicMock(stdout="commit message")
        result = pre_check.precheck_errors("HEAD~1", "HEAD")
        self.assertEqual(result, 1)
        mock_changed_files.assert_not_called()

    @patch.dict(os.environ, {**ALL_CHECKS_OFF, "CHECK_TABS": "1"})
    @patch("pre_check.git_file_entries")
    @patch("pre_check.changed_files")
    @patch("pre_check.PreCheck.find_tabs")
    @patch("pre_check.PreCheck.files_for_tabs")
    @patch("sys.stdout")
    def test_precheck_errors_tabs_fail(
        self, mock_stdout, mock_files, mock_find, mock_changed_files, mock_entries
    ):
        """Test precheck_errors() with tab characters"""
        mock_changed_files.return_value = {"file1.C"}
        mock_entries.return_value = [("100644", "file1.C")]
        mock_files.return_value = ["file1.C"]
        mock_find.return_value = ["file1.C"]
        result = pre_check.precheck_errors("HEAD~1", "HEAD")
        self.assertEqual(result, 1)
        mock_changed_files.assert_called_once_with("HEAD~1", "HEAD")
        mock_entries.assert_called_once_with(touched_files={"file1.C"})
        mock_files.assert_called_once_with()
        mock_find.assert_called_once_with(["file1.C"])

    @patch.dict(os.environ, {**ALL_CHECKS_OFF, "CHECK_TABS": "1"})
    @patch("pre_check.PreCheck.find_tabs")
    @patch("pre_check.subprocess.run")
    def test_precheck_errors_reports_touched_file_count(self, mock_run, mock_find):
        """Test precheck_errors() reports how many touched files are checked"""
        mock_run.side_effect = [
            subprocess.CompletedProcess(
                args=["git", "diff"],
                returncode=0,
                stdout="file1.C\x00file2.py\x00link.py\x00",
            ),
            subprocess.CompletedProcess(
                args=["git", "ls-files"],
                returncode=0,
                stdout=(
                    "100644 abc123 0\tfile1.C\x00"
                    "100644 abc123 0\tfile2.py\x00"
                    "120000 abc123 0\tlink.py\x00"
                ),
            ),
        ]
        mock_find.return_value = []

        with patch("sys.stdout", new=io.StringIO()) as mock_stdout:
            result = pre_check.precheck_errors("HEAD~1", "HEAD")

        self.assertEqual(result, 0)
        self.assertIn(
            "INFO: Checking 2 touched files in this PR.",
            mock_stdout.getvalue(),
        )
        self.assertEqual(mock_run.call_count, 2)


class TestMainFunction(unittest.TestCase):
    """Test main() entry point function"""

    @patch("pre_check.precheck_errors")
    def test_main_with_base_arg(self, mock_precheck):
        """Test main() with explicit --base argument"""
        mock_precheck.return_value = 0
        result = pre_check.main(["pre_check.py", "--base", "HEAD~1"])
        self.assertEqual(result, 0)
        mock_precheck.assert_called_once_with("HEAD~1", "HEAD")

    @patch("pre_check.subprocess.run")
    @patch("pre_check.precheck_errors")
    @patch("sys.stderr")
    def test_main_merge_base_origin_devel(self, mock_stderr, mock_precheck, mock_run):
        """Test main() uses origin/devel first when no --base provided"""
        mock_run.return_value = MagicMock(stdout="abc123\n", returncode=0)
        mock_precheck.return_value = 0

        result = pre_check.main(["pre_check.py"])
        self.assertEqual(result, 0)
        mock_precheck.assert_called_once_with("abc123", "HEAD")
        # Confirm origin/devel was tried first
        first_call_args = mock_run.call_args_list[0][0][0]
        self.assertIn("origin/devel", first_call_args)

    @patch("pre_check.subprocess.run")
    @patch("pre_check.precheck_errors")
    @patch("sys.stderr")
    def test_main_merge_base_fallback_to_local_devel(
        self, mock_stderr, mock_precheck, mock_run
    ):
        """Test main() falls back to local devel when origin/devel is unavailable"""
        mock_run.side_effect = [
            MagicMock(returncode=1, stdout="", stderr=""),  # origin/devel fails
            MagicMock(returncode=0, stdout="def456\n"),  # devel succeeds
        ]
        mock_precheck.return_value = 0

        result = pre_check.main(["pre_check.py"])
        self.assertEqual(result, 0)
        mock_precheck.assert_called_once_with("def456", "HEAD")

    @patch("pre_check.subprocess.run")
    @patch("sys.stderr")
    def test_main_merge_base_failure(self, mock_stderr, mock_run):
        """Test main() when both origin/devel and devel are unavailable"""
        mock_run.return_value = MagicMock(returncode=1, stdout="", stderr="")

        result = pre_check.main(["pre_check.py"])
        self.assertEqual(result, 1)


if __name__ == "__main__":
    unittest.main(verbosity=2, buffer=True)
