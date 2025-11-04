#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import unittest
import re
from TestHarness.util import FormatResultEntry, formatResult, colorText
from dataclasses import dataclass

# The default format used in TestHarness
DEFAULT_FORMAT = 'njcst'
# The default format used on CIVET
CIVET_FORMAT = 'tpnsc'

TEST_NAME = 'path/to/a.test'
TERM_COLS = 80

@dataclass
class Options:
    term_format: str = DEFAULT_FORMAT
    timing: bool = False
    extra_info: bool = False
    term_cols: int = TERM_COLS
    colored: bool = True

@dataclass
class JointStatus:
    status: str
    color: str
    message: str = None

class TestFormatResult(unittest.TestCase):
    @staticmethod
    def formatResult(name=TEST_NAME, entry={}, options={}, **kwargs) -> str:
        entry = FormatResultEntry(name=name, **entry)
        options = Options(**options)
        return formatResult(entry, options, **kwargs)

    @staticmethod
    def getLength(*args, term_cols=TERM_COLS) -> int:
        length = term_cols - len(args)
        for arg in args:
            length -= len(re.sub(r'\033\[\d+m', '', arg))
        return length

    @staticmethod
    def getDots(*args, colored=True, **kwargs):
        dots = '.' * TestFormatResult.getLength(*args, **kwargs)
        if colored:
            dots = colorText(dots, 'GREY')
        return dots

    def testNameOnly(self):
        # Name with colored dots after
        default = self.formatResult()
        n, j = default.split(' ')
        self.assertEqual(n, TEST_NAME)
        dots_format = self.getDots(n, colored=True)
        self.assertEqual(j, dots_format)

        # Name with non-colored dots after
        default_no_color = self.formatResult(options={'colored': False})
        n, j = default_no_color.split(' ')
        self.assertEqual(n, TEST_NAME)
        dots_format = self.getDots(n, colored=False)
        self.assertEqual(j, dots_format)

        # Name with even more non-colored dots after (extend term_cols)
        long_term_cols = 200
        default_longer = self.formatResult(options={'colored': False, 'term_cols': long_term_cols})
        _, j = default_longer.split(' ')
        dots_format = self.getDots(TEST_NAME, colored=False, term_cols=long_term_cols)
        self.assertEqual(j, dots_format)

        # Just a name
        civet = self.formatResult(options={'term_format': CIVET_FORMAT})
        self.assertEqual(civet, TEST_NAME)

        # Only named term format
        name_only = self.formatResult(options={'term_format': 'n'})
        self.assertEqual(name_only, TEST_NAME)

    def testNameAndTime(self):
        timing = 1.23456
        time_format = f'[{timing:.3f}s]'
        entry = {'timing': timing}
        options = {'timing': True}

        # Name, colored dots, timing
        default = self.formatResult(entry=entry, options=options)
        n, j, t = default.split(' ')
        self.assertEqual(n, TEST_NAME)
        self.assertEqual(t, time_format)
        dots_format = self.getDots(n, t, colored=True)
        self.assertEqual(j, dots_format)

        # Name, non-colored dots, timing
        options_no_color = options.copy()
        options_no_color['colored'] = False
        default_no_color = self.formatResult(entry=entry, options=options_no_color)
        _, j, _ = default_no_color.split(' ')
        dots_format = self.getDots(TEST_NAME, time_format, colored=False)
        self.assertEqual(j, dots_format)

        # Timing, name
        options_civet = options.copy()
        options_civet['term_format'] = CIVET_FORMAT
        civet = self.formatResult(entry=entry, options=options_civet)
        t, n = civet.split(' ')
        self.assertEqual(t, time_format)
        self.assertEqual(n, TEST_NAME)

        # Only name
        only_name = self.formatResult(entry=entry, options={'term_format': 'n'})
        self.assertEqual(only_name, TEST_NAME)

    def testNameAndStatus(self):
        status = 'FOO'
        status_spaces = ' ' * (8 - len(status))
        status_message = 'bar'
        status_color = 'CYAN'
        joint_status = JointStatus(status=status, color=status_color, message=status_message)
        entry = {'joint_status': joint_status, 'status_message': status_message}

        # name, dots, colored status message
        default = self.formatResult(entry=entry)
        _, _, s = default.split(' ')
        self.assertEqual(s, colorText(status_message, status_color))

        # name, dots, non-colored status message
        default_no_color = self.formatResult(entry=entry, options={'colored': False})
        _, _, s = default_no_color.split(' ')
        self.assertEqual(s, status_message)

        # right aligned colored status, name, colored status message
        civet = self.formatResult(entry=entry, options={'term_format': CIVET_FORMAT})
        p, s = civet.split(TEST_NAME)
        self.assertEqual(p, colorText(f'{status_spaces}{status}', status_color) + ' ')
        self.assertEqual(s, ' ' + colorText(status_message, status_color))

        # right aligned non-colored status, name, non-colored status message
        civet_no_color = self.formatResult(entry=entry, options={'term_format': CIVET_FORMAT, 'colored': False})
        p, s = civet_no_color.split(TEST_NAME)
        self.assertEqual(p, f'{status_spaces}{status} ')
        self.assertEqual(s, f' {status_message}')

        # Status message is same as status, so don't repeat: status and then name
        joint_status_same = JointStatus(status=status, color=None, message=status)
        entry_same = {'joint_status': joint_status_same, 'status_message': status}
        civet_same_message = self.formatResult(entry=entry_same, options={'term_format': CIVET_FORMAT})
        self.assertEqual(civet_same_message, f'{status_spaces}{status} {TEST_NAME}')

    def testCaveats(self):
        caveats = ['cool', 'story']
        caveats_joined = '[' + ','.join(caveats) + ']'
        caveat_color = 'RED'
        caveats_colored = colorText(caveats_joined, caveat_color)
        entry = {'caveats': caveats, 'caveat_color': caveat_color}
        options = {'term_format': 'c'}
        options_no_color = {**options, 'colored': False}

        # Just the caveats with color
        result = self.formatResult(entry=entry, options=options)
        self.assertEqual(result, caveats_colored)

        # Just the caveats without color
        result_no_color = self.formatResult(entry=entry, options=options_no_color)
        self.assertEqual(result_no_color, caveats_joined)

        # Name, dots, caveats colored
        default = self.formatResult(entry=entry)
        n, j, c = default.split(' ')
        self.assertEqual(n, TEST_NAME)
        self.assertEqual(c, caveats_colored)
        dots_format = self.getDots(n, caveats_colored)
        self.assertEqual(j, dots_format)

        # Name, non-colored dots, non-colored caveats
        default_no_color = self.formatResult(entry=entry, options={'colored': False})
        n, j, c = default_no_color.split(' ')
        self.assertEqual(n, TEST_NAME)
        self.assertEqual(c, caveats_joined)
        dots_format = self.getDots(n, caveats_joined, colored=False)
        self.assertEqual(j, dots_format)

        # Name, colored caveats
        civet = self.formatResult(entry=entry, options={'term_format': CIVET_FORMAT})
        n, c = civet.split(' ')
        self.assertEqual(n, TEST_NAME)
        self.assertEqual(c, caveats_colored)

        # Name, non-colored caveats
        civet = self.formatResult(entry=entry, options={'term_format': CIVET_FORMAT, 'colored': False})
        n, c = civet.split(' ')
        self.assertEqual(n, TEST_NAME)
        self.assertEqual(c, caveats_joined)

    def testLongCaveats(self):
        caveat = 'foo'
        caveats = [caveat] * 100
        caveats_joined = '[' + ','.join(caveats) + ']'

        # With caveats not last, they are shortened
        result = self.formatResult(entry={'caveats': caveats}, options={'term_format': 'cn', 'term_cols': 40})
        c, n = result.split(' ')
        self.assertEqual(c, f'[{",".join([caveat]*5)},...]')
        self.assertEqual(n, TEST_NAME)

        # With caveats not last but extra info, they are not shortened
        result = self.formatResult(entry={'caveats': caveats}, options={'term_format': 'cn', 'extra_info': True})
        c, n = result.split(' ')
        self.assertEqual(c, caveats_joined)
        self.assertEqual(n, TEST_NAME)

        # With caveats last, they are not shortened
        result = self.formatResult(entry={'caveats': caveats}, options={'term_format': 'nc'})
        n, c = result.split(' ')
        self.assertEqual(n, TEST_NAME)
        self.assertEqual(c, caveats_joined)

    def testUpperCase(self):
        # Upper case format designates making the thing upper
        result = self.formatResult(options={'term_format': 'N'})
        self.assertEqual(result, TEST_NAME.upper())

    def testOrder(self):
        # Set everything and test the ordering
        timing = 1
        status = 'COOLSTAT'
        status_message = 'bar'
        joint_status = JointStatus(status=status, color=None, message=status_message)
        caveats = ['foo', 'bar']
        result = self.formatResult(entry={'timing': timing,
                                          'joint_status': joint_status,
                                          'status_message': status_message,
                                          'caveats': caveats},
                                   options={'colored': False,
                                            'term_format': 'pnjcst',
                                            'timing': True})

        p, n, j, c, s, t = result.split(' ')
        self.assertEqual(p, status)
        self.assertEqual(n, TEST_NAME)
        self.assertEqual(j, self.getDots(p, n, c, s, t, colored=False))
        self.assertEqual(c, '[' + ','.join(caveats) + ']')
        self.assertEqual(s, status_message)
        self.assertEqual(t, '[1.000s]')

    def testTimingPrecision(self):
        def test(timing: float, formatted: str):
            result = self.formatResult(entry={'timing': timing}, options={'term_format': 't', 'timing': True})
            self.assertEqual(result, f'[{formatted}]')

        # Times are fixed length to 6 characters
        test(12345.1234, '12345s')
        test(2345.654, '2346s ')
        test(101.111, '101.1s')
        test(14.555, '14.55s')
        test(1.0001, '1.000s')
        test(0.02345, '0.023s')

if __name__ == '__main__':
    unittest.main()
