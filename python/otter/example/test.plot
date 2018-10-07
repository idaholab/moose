#
# Difference between the fission gas curves
# (this is up top to give the dependency resolver some work)
#
[diff]
  type = Difference
  source1 = fgr1
  source2 = fgr2
[]

#
# These blocks generate the actual plots (one is PDF, one is PNG)
#
[pdf1]
  type = Plot
  sources = 'fgr1 fgr2'
  file = fgr.png
  x_label = Time
  y_label = 'Fission gas release'
[]
[pdf2]
  type = Plot
  sources = 'diff'
  file = diff.png
  x_label = Time
[]

#
# Fission gas curves read from two different CSV files
# (note the different column names)
#
[fgr1]
  type = CSV
  file = out1.csv
  x_data = time
  y_data = fgr
[]
[fgr2]
  type = CSV
  file = out2.csv
  x_data = time
  y_data = release
[]
