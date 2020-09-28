import csv
import numpy as np

##
# Reads data from a CSV file into dictionary of numpy arrays
#
# @param[in] inputfile   Name of the CSV file to read
#
# @return dictionary of numpy arrays
#
def readCSVFile(inputfile):
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

  return data
