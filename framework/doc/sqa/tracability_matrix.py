#!/usr/bin/env python

import re, os, sys
from reportlab.lib import colors
from reportlab.lib.units import cm
from reportlab.lib.pagesizes import A4, inch, landscape
from reportlab.platypus import SimpleDocTemplate, Table, TableStyle, Paragraph
from reportlab.lib.styles import getSampleStyleSheet


def buildTable(data):
  doc = SimpleDocTemplate("MOOSE_requirements_tracability.pdf", pagesize=A4, rightMargin=30,leftMargin=30, topMargin=30,bottomMargin=18)
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
def extractRequirements(filename):
  f = open(filename)
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


def extractTestedRequirements(data):
  # Here we will use the TestHarness to find all of the
  # test files where we can look for tested requirements.
  # Assume SQA docs are located in <MOOSE_DIR>/framework/doc/sqa
  MOOSE_DIR = os.path.abspath(os.path.join('..', '..', '..'))
  #### See if MOOSE_DIR is already in the environment instead
  if os.environ.has_key("MOOSE_DIR"):
    MOOSE_DIR = os.environ['MOOSE_DIR']

  test_app_name = 'moose_test'
  test_app_dir = os.path.join(MOOSE_DIR, 'test')

  # Set the current working directory to test_app_dir
  saved_cwd = os.getcwd()
  os.chdir(test_app_dir)

  sys.path.append(os.path.join(MOOSE_DIR, 'python'))
  import path_tool
  path_tool.activate_module('TestHarness')

  from TestHarness import TestHarness
  from Tester import Tester

  # Build the TestHarness object here
  harness = TestHarness(sys.argv, test_app_name, MOOSE_DIR)
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
    m = re.search(r'@Requirement\s+([\w\.]+)', text)
    if m != None:
      requirement = m.group(1)
      if requirement not in data:
        print 'Unable to find referenced requirement "' + requirement + '" in ' + input_path
      else:
        data[requirement][2].add(os.path.relpath(input_path, MOOSE_DIR))

  os.chdir(saved_cwd)


if __name__ == "__main__":
  data = extractRequirements("SoftwareRequirements.tex")

  extractTestedRequirements(data)

  buildTable(data)
