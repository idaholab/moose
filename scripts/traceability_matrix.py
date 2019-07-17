#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import re, os, sys, argparse
from reportlab.lib import colors
from reportlab.lib.pagesizes import A4, landscape
from reportlab.platypus import SimpleDocTemplate, Table, TableStyle, Paragraph
from reportlab.lib.styles import getSampleStyleSheet


def buildTable(args, data):
    # Change to what ever directory the user was in at
    # the time they executed this script
    os.chdir(args.cwd)

    doc = SimpleDocTemplate(args.application_name + "_requirements_traceability.pdf", pagesize=A4, rightMargin=30,leftMargin=30, topMargin=30,bottomMargin=18)
    doc.pagesize = landscape(A4)
    elements = []

    #Configure style and word wrap
    s = getSampleStyleSheet()
    s = s["BodyText"]
    s.wordWrap = 'CJK'

    pdf_data = [["Requirement", "Description", "Test Case(s)"]]

    #TODO: Need a numerical sort here
    keys = sorted(data.keys())

    for key in keys:
        data[key][2] = '\n'.join(data[key][2])
        pdf_data.append([Paragraph(cell, s) for cell in data[key]])


    # Build the Table and Style Information
    tableThatSplitsOverPages = Table(pdf_data, repeatRows=1)
    tableThatSplitsOverPages.hAlign = 'LEFT'
    tblStyle = TableStyle([('TEXTCOLOR',(0,0),(-1,-1),colors.black),
                           ('VALIGN',(0,0),(-1,-1),'TOP'),
                           ('LINEBELOW',(0,0),(-1,-1),1,colors.black),
                           ('INNERGRID', (0,0), (-1,-1),1,colors.black),
                           ('BOX',(0,0),(-1,-1),1,colors.black),
                           ('BOX',(0,0),(0,-1),1,colors.black)])
    tblStyle.add('BACKGROUND',(0,0),(-1,-1),colors.lightblue)
    tblStyle.add('BACKGROUND',(0,1),(-1,-1),colors.white)
    tableThatSplitsOverPages.setStyle(tblStyle)
    elements.append(tableThatSplitsOverPages)

    doc.build(elements)

##########
# This routine extracts the requirements from the "SoftwareREquirements.tex" document
##########
def extractRequirements(args):
    f = open(args.requirements_path)
    text = f.read()
    f.close()

    data = {} # [['Requirement Number', 'Requirement Description']]

    # Extract all of the tables
    for table in re.finditer(r'\\begin{tabular}.*?\\end{tabular}', text, re.S):
        if re.search(r'Number.*?Requirement Description', table.group(0)) != None:

            header_row = True

            # Look for this pattern:
            # F1.10 & Description
            for req in re.finditer(r'([\w\.]+)\s+&\s+(.*?)\s*\\\\', table.group(0)):

                if header_row:
                    header_row = False

                    # Skip the header
                    continue
                else:
                    # Store the requirment number and description in the data structure
                    #data.append([req.group(1), req.group(2)])
                    data[req.group(1)] = [req.group(1), req.group(2), set()]

    return data


def extractTestedRequirements(args, data):
    # Here we will use the TestHarness to find all of the
    # test files where we can look for tested requirements.
    # Assume SQA docs are located in <MOOSE_DIR>/framework/doc/sqa
    test_app_name = args.application_name
    test_app_dir = os.path.join(args.application_path)

    #### TODO
    # figure out a cleaner way to set this up
    # If test_app_name is framework, we need to reword some things
    if test_app_name == 'framework':
        test_app_name = 'moose_test'
        test_app_dir = os.path.join(args.moose_dir, 'test')

    # Set the current working directory to test_app_dir
    saved_cwd = os.getcwd()
    os.chdir(test_app_dir)

    sys.path.append(os.path.join(args.moose_dir, 'python'))

    from TestHarness import TestHarness

    # Build the TestHarness object here
    harness = TestHarness([], args.moose_dir, test_app_name)

    # Tell it to parse the test files only, not run them
    harness.findAndRunTests(find_only=True)

    # Now retrieve all of the accumlated Testers from the TestHarness warehouse
    testers = harness.warehouse.getAllObjects()
    for tester in testers:
        print tester.specs['test_name']
        input_filename = tester.getInputFile()
        if input_filename == None:
            continue

        input_path = os.path.join(tester.specs['test_dir'], input_filename)
        if not os.path.isfile(input_path):
            continue

        # Read the MOOSE input file
        f = open(os.path.join(tester.specs['test_dir'], tester.getInputFile()))
        text = f.read()
        f.close()

        # See if the file maps to a requirement (e.g. @Requirement)
        for req in re.finditer(r'@Requirement\s+([\w\.]+)', text):
            requirement = req.group(1)
            if requirement not in data:
                print 'Unable to find referenced requirement "' + requirement + '" in ' + input_path
            else:
                data[requirement][2].add(os.path.relpath(input_path, args.moose_dir))

    os.chdir(saved_cwd)

def verifyArguments(args):
    # Verify supplied arguments
    if args.application is None or os.path.exists(args.application) is False:
        # Before we error, lets verfiy if current parent directory matches
        # the application they were supplying:
        if os.path.basename(os.getcwd()) != args.application:
            print 'You must specify a path to the application you wish to build an SQA documentation for.'
            sys.exit(1)
        else:
            args.application = os.getcwd()
    else:
        if os.path.exists(os.path.join(args.application, 'doc/sqa', args.requirements)) is False:
            print 'I could not find ', os.path.join(os.path.abspath(args.application), 'doc/sqa', args.requirements), 'file.' \
              '\nPlease see the directory:', os.path.join(os.path.abspath(os.path.dirname(sys.argv[0])), '..', 'framework/doc/sqa'), 'for a working example'
            sys.exit(1)

    args.application_path = os.path.abspath(args.application)
    args.requirements_path = os.path.join(args.application_path, 'doc/sqa', args.requirements)
    args.application_name = os.path.split(args.application_path)[1]

    # Set the current working directory to this script location
    # We to do this _after_ discovering application path in case
    # the user supplied a relative path instead of an absolute
    # path
    args.cwd = os.getcwd()
    os.chdir(os.path.abspath(os.path.dirname(sys.argv[0])))

    # Set MOOSE_DIR to parent directory (were sitting
    # in moose/scripts at the moment)
    args.moose_dir = os.path.abspath('..')

    return args

def parseArguments():
    parser = argparse.ArgumentParser(description='Build SQA Documentation')
    parser.add_argument('--application', '-a', metavar='application', help='Path to application you wish to build SQA documentation for')
    parser.add_argument('--requirements', '-r', nargs=1, default='SoftwareRequirements.tex', metavar='requirements.tex', help='Default: application_path/doc/sqa/%(default)s')
    return verifyArguments(parser.parse_args())

if __name__ == "__main__":
    # Parse supplied arguments
    args = parseArguments()

    # Get requirements from tex file and build SQA
    data = extractRequirements(args)
    extractTestedRequirements(args, data)
    buildTable(args, data)
