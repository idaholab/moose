#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import csv
import os
import numpy as np
import xml.etree.ElementTree as ET

##
# Reads data from a CSV file into dictionary of numpy arrays
#
# @param[in] inputfile   Name of the CSV file to read
#
# @return dictionary of numpy arrays
#
def readCSVFile(inputfile):
  print("Reading '" + inputfile + "'...", end='', flush=True)

  first_line_processed = False
  data = dict()
  variable_name_to_index = dict()

  # read data from input file into lists
  with open(inputfile) as csvfile:
    reader = csv.reader(csvfile, delimiter=',')
    for row in reader:
      if (row):
        if (first_line_processed):
          for key in data:
            try:
              value = float(row[variable_name_to_index[key]])
            except:
              value = row[variable_name_to_index[key]]
            data[key].append(value)
        else:
          for i,entry in enumerate(row):
            variable_name_to_index[entry] = i
            data[entry] = list()
          first_line_processed = True

  # convert lists to numpy arrays
  for key in data:
    data[key] = np.array(data[key])

  print('Done.')

  return data

##
# Reads an XML file output by a MOOSE-based application
#
# XML is the preferred output for vector post-processors (vpps).
# This function returns a dictionary of dictionaries of lists of lists, with the following structure:
#   data[<vpp_name>][<vector_name>][<timestep_index>][<index_within_vector>]
#
# @param[in] filename  Name of file to read
#
def readMOOSEXML(filename):
  print("Reading '" + filename + "'...", end='', flush=True)

  tree = ET.parse(filename)
  root = tree.getroot()
  data = dict()
  for timestep_elem in root:
    for vector_elem in timestep_elem:
      vpp = vector_elem.attrib['object']
      vector_name = vector_elem.attrib['name']
      if vpp not in data:
        data[vpp] = dict()
      if vector_name not in data[vpp]:
        data[vpp][vector_name] = []
      data[vpp][vector_name].append([float(elem) for elem in vector_elem.text.split()])

  print('Done.')

  return data

##
# Writes data to a CSV file
#
# @param[in] data       Dictionary of lists to output to file
# @param[in] filename   Name of the output file
# @param[in] append     Append data instead of over-writing it?
# @param[in] precision  Precision of entries in file
#
def writeCSVFile(data, filename, append=False, precision=5):
  output_file_exists = os.path.isfile(filename)

  if append:
    open_mode = 'a'

    # check that headers are the same
    if output_file_exists:
      existing_data = readCSVFile(filename)
      if set(data.keys()) != set(existing_data.keys()):
        raise Exception('The data to be appended does not have the same keys as the existing CSV.')
  else:
    open_mode = 'w'

  with open(filename, open_mode) as csvfile:
    writer = csv.writer(csvfile)

    keys = list(data.keys())

    # write header row if not appending
    if not append or not output_file_exists:
      writer.writerow(keys)

    # create number format string
    format_string = "%." + str(precision) + "e"

    # write data
    for i, item in enumerate(data[keys[0]]):
      row_data = list()
      for key in data:
        value = data[key][i]
        if isinstance(value, (int, float)):
          value_string = format_string % value
        else:
          value_string = value
        row_data.append(value_string)

      writer.writerow(row_data)
