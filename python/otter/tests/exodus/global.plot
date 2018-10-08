#
# Global variable from exodus file
#
[total_c]
  type = ExodusGlobal
  file = 'testexodus_out.e'
  y_data = c_total
[../]

#
# Write out CSV data
#
[csv]
  type = WriteCSV
  sources = 'total_c'
  file = global.csv
  x_name = t
[../]
