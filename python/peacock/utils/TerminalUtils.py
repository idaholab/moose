#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import re
import traceback

TERMINAL_TO_HTML_COLOR_MAP = { 30: "black",
        31: "red",
        32: "green",
        33: "yellow",
        34: "blue",
        35: "magenta",
        36: "cyan",
        37: "white",
        }

def terminalOutputToHtml(output):
    """
    Converts output with terminal codes into HTML.
    Input:
        output[str]: Text that might contain terminal codes
    Return:
        str: any terminal codes replaced by html and "&", ">", and "<" replaced.
    """
    html_tmp = output.replace("&", "&amp;")
    html_tmp = html_tmp.replace("<", "&lt;")
    html_tmp = html_tmp.replace(">", "&gt;")
    try:
        html_tmp = re.sub("\33\[39m", '</span>', html_tmp)
        html_tmp = re.sub("(\33\[1m)*\33\[(\d{1,2})m", _startSpan, html_tmp)
    except Exception:
        print("Got exception while trying to convert terminal codes to html:")
        print(traceback.format_exc())
    return html_tmp

def _startSpan(match):
    """
    Starts a <span> and set its color to the corresponding terminal code
    Input:
        match: a "re" match object resulting from the regular expression in terminalOutputToHtml
    Return:
        str: A <span>
    """
    html_color = None
    color_code = int(match.group(2))
    html_color = TERMINAL_TO_HTML_COLOR_MAP.get(color_code)

    if html_color:
        return '<span style="color:%s;">' % html_color
    else:
        return "<span>"
