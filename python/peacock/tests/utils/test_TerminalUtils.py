#!/usr/bin/env python
from peacock.utils import TerminalUtils
from peacock.utils import Testing

class Tests(Testing.PeacockTester):
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
