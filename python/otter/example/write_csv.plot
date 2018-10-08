#
# Fission gas curves read from two different CSV files
# (note the different column names)
#
[./fgr1]
  type = CSV
  file = out1.csv
  x_data = time
  y_data = fgr
[]
[./fgr2]
  type = CSV
  file = out2.csv
  x_data = time
  y_data = release
[]

#
# Write combined CSV file
#
[./write_csv]
  type = WriteCSV
  file = union.csv
  x_data = time
  sources = 'fgr1 fgr2'
[]
