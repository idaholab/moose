#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import os
import sys
import argparse
import logging

moose_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(moose_dir, 'python'))
from mooseutils import colorText
import moosesqa

def get_options():
    """Command-line options."""
    parser = argparse.ArgumentParser(description='SQA document checking tool.')
    parser.add_argument('--config', type=str,
                        default=os.path.join(os.path.dirname(os.path.abspath(__file__)), '../modules/doc/sqa_reports.yml'),
                        help="The YAML config file for performing SQA checks.")
    parser.add_argument('--reports', nargs='+', default=['doc', 'req', 'app'], choices=['doc', 'req', 'app'],
                        help='Select the reports to produce.')

    parser.add_argument('--skip', nargs='+', default=[],
                        help="(deprecated) This options is being removed.")

    return parser.parse_args()

def main(config, **kwargs):
    """Print the SQA information reports."""
    doc_reports, req_reports, app_reports = moosesqa.get_sqa_reports(config, **kwargs)
    status = print_reports('MooseApp', app_reports, 0)
    status = print_reports('Document', doc_reports, status)
    status = print_reports('Requirement', req_reports, status)
    return status

def print_reports(title, reports, status):
    print(colorText('\n{0}\n{1} REPORT(S):\n{0}\n'.format('-'*80, title.upper()), 'MAGENTA'), end='', flush=True)
    if reports:
        for report in reports:
            print(report.getReport(), '\n')
            status = report.status if status < report.status else status
    else:
        print('No results')
    return status

if __name__ == '__main__':
    print("This script is deprecated and will be removed, use moosedocs.py check.")

    opt = get_options()
    if opt.skip:
        print('No checks performed, the --skip argument is being removed, please update to use the config file.')
        status = 0
    else:
        kwargs = {'app_report':'app' in opt.reports, 'doc_report':'doc' in opt.reports, 'req_report':'req' in opt.reports}
        status = main(opt.config, **kwargs)

    critical = moosesqa.SilentRecordHandler.COUNTS[logging.CRITICAL]
    error = moosesqa.SilentRecordHandler.COUNTS[logging.ERROR]
    warning = moosesqa.SilentRecordHandler.COUNTS[logging.WARNING]
    print('CRITICAL:{} ERROR:{} WARNING:{}\nExit Status: {}'.format(critical, error, warning, int(status > 1)))
    sys.exit(status > 1) # 1 is warning; 2 is error
