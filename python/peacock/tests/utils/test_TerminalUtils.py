#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from peacock.utils import TerminalUtils
from peacock.utils import Testing
from PyQt5 import QtWidgets

class Tests(Testing.PeacockTester):
    qapp = QtWidgets.QApplication([])

    def test_convert(self):
        output = "\33[1m\33[31mred <&> text\33[39m"
        html_output = TerminalUtils.terminalOutputToHtml(output)
        self.assertEqual('<span style="color:red;">red &lt;&amp;&gt; text</span>', html_output)

        output = "\n\33[1m\33[31m\nfoo\nred text\n\33[39m"
        html_output = TerminalUtils.terminalOutputToHtml(output)
        self.assertEqual('\n<span style="color:red;">\nfoo\nred text\n</span>', html_output)

        # bad color code
        output = "\33[1m\33[10munknown color\33[39m"
        html_output = TerminalUtils.terminalOutputToHtml(output)
        self.assertEqual('<span>unknown color</span>', html_output)

if __name__ == '__main__':
    Testing.run_tests()
