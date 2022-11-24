#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import sys
import subprocess
import datetime

def main():
    today = datetime.date.today()
    month, year = (today.month - 1, today.year) if today.month > 1 else (12, today.year-1)
    after = datetime.date(year, month, 1).strftime('%Y-%m-%d')
    before = datetime.date(today.year, today.month, 1).strftime('%Y-%m-%d')

    author = subprocess.check_output(['git', 'config', 'user.name'], encoding='utf8').strip('\n')
    cmd = ['git', 'log', '--no-merges', '--author', author, '--after', after, '--before', before]
    print(' '.join(cmd))
    return subprocess.run(cmd)

if __name__ == '__main__':
    proc = main()
    sys.exit(proc.returncode)
