#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import sys
import unittest
from unittest.mock import patch, MagicMock, mock_open
import subprocess
import tempfile

# Add the scripts directory to the path so we can import pre_check
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))
import pre_check


class TestHelpers(unittest.TestCase):
    """Test helper functions"""

    @patch('pre_check.subprocess.run')
    def test_run_success(self, mock_run):
        """Test the run() helper function"""
        mock_run.return_value = subprocess.CompletedProcess(
            args=['git', 'status'],
            returncode=0,
            stdout='output'
        )
        result = pre_check.run(['git', 'status'])
        self.assertEqual(result.returncode, 0)
        self.assertEqual(result.stdout, 'output')

    @patch('pre_check.subprocess.run')
    def test_git_files(self, mock_run):
        """Test git_files() function"""
        mock_run.return_value = subprocess.CompletedProcess(
            args=['git', 'ls-files'],
            returncode=0,
            stdout='file1.py\x00file2.C\x00contrib/skip.py\x00dir/contrib/skip.C\x00file3.h\x00'
        )
        files = pre_check.git_files('*.py', '*.[Ch]')
        self.assertIn('file1.py', files)
        self.assertIn('file2.C', files)
        self.assertIn('file3.h', files)
        self.assertNotIn('contrib/skip.py', files)
        self.assertNotIn('dir/contrib/skip.C', files)

    def test_read_text_bytes(self):
        """Test read_text_bytes() function"""
        with patch('builtins.open', mock_open(read_data=b'test content')):
            result = pre_check.read_text_bytes('dummy.txt')
            self.assertEqual(result, b'test content')

        # Test error handling
        with patch('builtins.open', side_effect=IOError('error')):
            result = pre_check.read_text_bytes('dummy.txt')
            self.assertEqual(result, b'')

    def test_read_text(self):
        """Test read_text() function"""
        with patch('builtins.open', mock_open(read_data='test content')):
            result = pre_check.read_text('dummy.txt')
            self.assertEqual(result, 'test content')


class TestTicketReferences(unittest.TestCase):
    """Test ticket reference detection"""

    @patch('pre_check.run')
    def test_ticket_references_found(self, mock_run):
        """Test detection of ticket references in commit messages"""
        mock_run.return_value = MagicMock(
            stdout='Commit 1\nFix issue #1234\nCommit 2\nmoose/issues/5678\nCommit 3\nmoose/pull/9012'
        )
        result = pre_check.ticket_references('HEAD~3', 'HEAD')
        self.assertIn('#1234', result)
        self.assertIn('moose/issues/5678', result)
        self.assertIn('moose/pull/9012', result)

    @patch('pre_check.run')
    def test_ticket_references_not_found(self, mock_run):
        """Test when no ticket references are found"""
        mock_run.return_value = MagicMock(
            stdout='Commit 1\nNo ticket here\nCommit 2\nAnother commit'
        )
        result = pre_check.ticket_references('HEAD~2', 'HEAD')
        self.assertEqual(result, '')


class TestFileSetHelpers(unittest.TestCase):
    """Test file set helper functions"""

    @patch('pre_check.git_files')
    def test_files_for_tabs_except_input(self, mock_git_files):
        """Test files_for_tabs_except_input()"""
        mock_git_files.return_value = ['file1.C', 'file2.py']
        result = pre_check.files_for_tabs_except_input()
        mock_git_files.assert_called_once_with("*.[Ch]", "*.py")
        self.assertEqual(result, ['file1.C', 'file2.py'])

    @patch('pre_check.git_files')
    def test_files_for_tabs(self, mock_git_files):
        """Test files_for_tabs()"""
        mock_git_files.return_value = ['file1.C', 'file2.i', 'file3.py']
        result = pre_check.files_for_tabs()
        mock_git_files.assert_called_once_with("*.[Chi]", "*.py")

    @patch('pre_check.git_files')
    def test_files_for_whitespace_cpp_enabled(self, mock_git_files):
        """Test files_for_whitespace() with C++ files"""
        mock_git_files.return_value = ['file1.C', 'file2.py']
        result = pre_check.files_for_whitespace("1", "0")
        mock_git_files.assert_called_once_with("*.[Cchi]", "*.py")

    @patch('pre_check.git_files')
    def test_files_for_whitespace_fortran_enabled(self, mock_git_files):
        """Test files_for_whitespace() with Fortran files"""
        mock_git_files.return_value = ['file1.F', 'file2.f90']
        result = pre_check.files_for_whitespace("0", "1")
        mock_git_files.assert_called_once_with("*.i", "*.py", "*.[FfH]", "*.f90", "*.F90", "*.FF90")


class TestIndividualChecks(unittest.TestCase):
    """Test individual check functions"""

    @patch('pre_check.read_text')
    def test_find_tabs(self, mock_read_text):
        """Test find_tabs() function"""
        mock_read_text.side_effect = lambda f: 'line with\ttab' if f == 'file1.C' else 'no tabs'
        result = pre_check.find_tabs(['file1.C', 'file2.C'])
        self.assertEqual(result, ['\tfile1.C'])

    @patch('pre_check.git_files')
    @patch('pre_check.read_text')
    def test_banned_keywords_cout(self, mock_read_text, mock_git_files):
        """Test banned_keywords() detects std::cout"""
        mock_git_files.return_value = ['file1.C']
        mock_read_text.return_value = 'std::cout << "hello"'
        result = pre_check.banned_keywords()
        self.assertEqual(result, ['\tfile1.C'])

    @patch('pre_check.git_files')
    @patch('pre_check.read_text')
    def test_banned_keywords_printf(self, mock_read_text, mock_git_files):
        """Test banned_keywords() detects printf"""
        mock_git_files.return_value = ['file1.C']
        mock_read_text.return_value = 'printf("test")'
        result = pre_check.banned_keywords()
        self.assertEqual(result, ['\tfile1.C'])

    @patch('pre_check.git_files')
    @patch('pre_check.read_text')
    def test_banned_keywords_sleep_exception(self, mock_read_text, mock_git_files):
        """Test banned_keywords() allows sleep in SlowProblem.C"""
        mock_git_files.return_value = ['SlowProblem.C', 'other.C']
        mock_read_text.side_effect = lambda f: 'sleep(1)' if f in ['SlowProblem.C', 'other.C'] else ''
        result = pre_check.banned_keywords()
        self.assertEqual(result, ['\tother.C'])

    @patch('pre_check.git_files')
    @patch('pre_check.read_text')
    def test_banned_funcs(self, mock_read_text, mock_git_files):
        """Test banned_funcs() detects deprecated MOOSE functions"""
        mock_git_files.return_value = ['file1.C']
        mock_read_text.return_value = 'mooseError2("test")'
        result = pre_check.banned_funcs()
        self.assertEqual(result, ['\tfile1.C'])

    @patch('pre_check.git_files')
    @patch('pre_check.read_text')
    def test_classified_keywords(self, mock_read_text, mock_git_files):
        """Test classified_keywords() detects proprietary/classified"""
        mock_git_files.return_value = ['file1.C', 'pre_check.py']
        mock_read_text.side_effect = lambda f: 'p r o p r i e t a r y' if f == 'file1.C' else 'proprietary'
        result = pre_check.classified_keywords()
        self.assertEqual(result, ['\tfile1.C'])

    @patch('builtins.open', new_callable=mock_open, read_data='line1  \nline2\n')
    def test_trailing_whitespace_files(self, mock_file):
        """Test trailing_whitespace_files() detects trailing spaces"""
        result = pre_check.trailing_whitespace_files(['file1.py'])
        self.assertEqual(result, ['\tfile1.py'])

    @patch('builtins.open', new_callable=mock_open, read_data='line1\nline2\n')
    def test_trailing_whitespace_files_no_whitespace(self, mock_file):
        """Test trailing_whitespace_files() with no trailing spaces"""
        result = pre_check.trailing_whitespace_files(['file1.py'])
        self.assertEqual(result, [])

    @patch('pre_check.git_files')
    def test_no_newline_at_eof_files(self, mock_git_files):
        """Test no_newline_at_eof_files() detects missing newline"""
        mock_git_files.return_value = ['file1.py', 'file2.py']

        with patch('builtins.open', mock_open(read_data=b'content')):
            result = pre_check.no_newline_at_eof_files()
            self.assertEqual(result, ['\tfile1.py', '\tfile2.py'])

    @patch('pre_check.run')
    @patch('os.stat')
    @patch('os.access')
    def test_find_bad_executables(self, mock_access, mock_stat, mock_run):
        """Test find_bad_executables() detects incorrectly executable files"""
        mock_run.return_value = MagicMock(stdout='file1.txt\nscript.py\nREADME.md')
        mock_stat.return_value = MagicMock(st_mode=0o100755)
        mock_access.return_value = True

        result = pre_check.find_bad_executables()
        self.assertIn('\tfile1.txt', result)
        self.assertIn('\tREADME.md', result)

    @patch('pre_check.files_for_headers')
    @patch('pre_check.read_text')
    def test_include_guard_files(self, mock_read_text, mock_headers):
        """Test include_guard_files() detects old-style guards"""
        mock_headers.return_value = ['header.h']
        mock_read_text.return_value = '#ifndef HEADER_H\n#define HEADER_H\ncontent\n#endif'
        result = pre_check.include_guard_files()
        self.assertEqual(result, ['\theader.h'])

    @patch('pre_check.git_files')
    @patch('pre_check.read_text_bytes')
    def test_windows_line_endings(self, mock_read_bytes, mock_git_files):
        """Test windows_line_endings() detects CRLF"""
        mock_git_files.return_value = ['file1.C']
        mock_read_bytes.return_value = b'line1\r\nline2\r\n'
        result = pre_check.windows_line_endings()
        self.assertEqual(result, ['\tfile1.C'])


class TestUnicodeChecks(unittest.TestCase):
    """Test Unicode character checking"""

    def test_is_allowed_manual_ascii(self):
        """Test _is_allowed_manual() with ASCII characters"""
        self.assertTrue(pre_check._is_allowed_manual('a'))
        self.assertTrue(pre_check._is_allowed_manual('Z'))
        self.assertTrue(pre_check._is_allowed_manual('0'))

    def test_is_allowed_manual_latin_extended(self):
        """Test _is_allowed_manual() with Latin Extended characters"""
        self.assertTrue(pre_check._is_allowed_manual('é'))  # Latin-1
        self.assertTrue(pre_check._is_allowed_manual('ł'))  # Latin Extended-A
        self.assertTrue(pre_check._is_allowed_manual('ș'))  # Latin Extended-A

    def test_is_allowed_manual_greek(self):
        """Test _is_allowed_manual() with Greek characters"""
        self.assertTrue(pre_check._is_allowed_manual('α'))
        self.assertTrue(pre_check._is_allowed_manual('Ω'))

    def test_is_allowed_manual_math_symbols(self):
        """Test _is_allowed_manual() with math symbols"""
        self.assertTrue(pre_check._is_allowed_manual('∇'))  # Nabla
        self.assertTrue(pre_check._is_allowed_manual('∫'))  # Integral
        self.assertTrue(pre_check._is_allowed_manual('±'))  # Plus-minus
        self.assertTrue(pre_check._is_allowed_manual('∂'))  # Partial

    def test_is_allowed_manual_superscripts(self):
        """Test _is_allowed_manual() with superscripts/subscripts"""
        self.assertTrue(pre_check._is_allowed_manual('²'))
        self.assertTrue(pre_check._is_allowed_manual('₃'))

    def test_is_allowed_manual_disallowed(self):
        """Test _is_allowed_manual() with disallowed characters"""
        self.assertFalse(pre_check._is_allowed_manual('😀'))  # Emoji
        self.assertFalse(pre_check._is_allowed_manual('中'))  # CJK

    def test_disallowed_spans(self):
        """Test _disallowed_spans() function"""
        text = 'hello 😀 world'
        spans = pre_check._disallowed_spans(text)
        self.assertEqual(len(spans), 1)
        self.assertEqual(spans[0][0], 6)  # Position of emoji
        self.assertEqual(spans[0][1], '😀')

    def test_get_line_col(self):
        """Test _get_line_col() function"""
        text = 'line1\nline2\nline3'
        line, col = pre_check._get_line_col(text, 0)
        self.assertEqual((line, col), (1, 1))

        line, col = pre_check._get_line_col(text, 6)
        self.assertEqual((line, col), (2, 1))

    @patch('pre_check.git_files')
    @patch('pre_check.read_text')
    def test_unicode_files(self, mock_read_text, mock_git_files):
        """Test unicode_files() detects disallowed Unicode"""
        mock_git_files.return_value = ['file1.C']
        mock_read_text.return_value = 'hello 😀 world'
        bad_files, locations = pre_check.unicode_files()
        self.assertEqual(bad_files, ['\tfile1.C'])
        self.assertEqual(len(locations), 1)
        self.assertIn('file1.C:1:7', locations[0])


class TestPrecheckErrors(unittest.TestCase):
    """Test main precheck_errors() function"""

    @patch.dict(os.environ, {
        'CHECK_TICKET_REFERENCE': '0',
        'CHECK_KEYWORDS': '0',
        'CHECK_EOF': '0',
        'CHECK_EXECUTABLES': '0',
        'CHECK_WHITESPACE': '0',
        'CHECK_TABS': '0',
        'CHECK_CLASSIFIED': '0',
        'CHECK_UNICODE': '0',
        'CHECK_INCLUDE_GUARDS': '0',
        'CHECK_WINDOWS_FILES': '0'
    })
    def test_precheck_errors_all_disabled(self):
        """Test precheck_errors() with all checks disabled"""
        result = pre_check.precheck_errors('HEAD~1', 'HEAD')
        self.assertEqual(result, 0)

    @patch.dict(os.environ, {'CHECK_TICKET_REFERENCE': '1'})
    @patch('pre_check.ticket_references')
    @patch('sys.stdout')
    def test_precheck_errors_ticket_pass(self, mock_stdout, mock_ticket):
        """Test precheck_errors() with valid ticket reference"""
        mock_ticket.return_value = 'Fix #1234'
        result = pre_check.precheck_errors('HEAD~1', 'HEAD')
        self.assertEqual(result, 0)

    @patch.dict(os.environ, {'CHECK_TICKET_REFERENCE': '1'})
    @patch('pre_check.ticket_references')
    @patch('pre_check.run')
    @patch('sys.stdout')
    def test_precheck_errors_ticket_fail(self, mock_stdout, mock_run, mock_ticket):
        """Test precheck_errors() without ticket reference"""
        mock_ticket.return_value = ''
        mock_run.return_value = MagicMock(stdout='commit message')
        result = pre_check.precheck_errors('HEAD~1', 'HEAD')
        self.assertEqual(result, 1)

    @patch.dict(os.environ, {'CHECK_TABS': '1'})
    @patch('pre_check.find_tabs')
    @patch('pre_check.files_for_tabs')
    @patch('sys.stdout')
    def test_precheck_errors_tabs_fail(self, mock_stdout, mock_files, mock_find):
        """Test precheck_errors() with tab characters"""
        mock_files.return_value = ['file1.C']
        mock_find.return_value = ['\tfile1.C']
        result = pre_check.precheck_errors('HEAD~1', 'HEAD')
        self.assertEqual(result, 1)


class TestMainFunction(unittest.TestCase):
    """Test main() entry point function"""

    @patch('pre_check.precheck_errors')
    def test_main_with_args(self, mock_precheck):
        """Test main() with explicit arguments"""
        mock_precheck.return_value = 0
        result = pre_check.main(['pre_check.py', 'HEAD~1', 'HEAD'])
        self.assertEqual(result, 0)
        mock_precheck.assert_called_once_with('HEAD~1', 'HEAD')

    @patch('pre_check.run')
    @patch('pre_check.precheck_errors')
    @patch('sys.stderr')
    def test_main_merge_base_success(self, mock_stderr, mock_precheck, mock_run):
        """Test main() using merge-base when no args provided"""
        mock_run.return_value = MagicMock(stdout='abc123\n', returncode=0)
        mock_precheck.return_value = 0

        result = pre_check.main(['pre_check.py'])
        self.assertEqual(result, 0)
        mock_precheck.assert_called_once_with('abc123', 'HEAD')

    @patch('pre_check.run')
    @patch('sys.stderr')
    def test_main_merge_base_failure(self, mock_stderr, mock_run):
        """Test main() when merge-base fails"""
        mock_run.side_effect = subprocess.CalledProcessError(1, 'git')

        result = pre_check.main(['pre_check.py'])
        self.assertEqual(result, 1)


if __name__ == '__main__':
    unittest.main(verbosity=2, buffer=True)
