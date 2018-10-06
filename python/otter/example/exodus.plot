#
# Global variable from exodus file
#
[./c_total]
  type = ExodusGlobal
  file = testexodus_out.e
  y_data = c_total
[../]

#
# These blocks generate the actual plot
#
[./pdf]
  type = Plot
  sources = 'c_total'
  file = exodus.pdf
  x_label = Time
  y_label = 'Total solute'
[../]
