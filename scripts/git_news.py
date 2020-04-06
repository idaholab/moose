#!/usr/bin/env python
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
