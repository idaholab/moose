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

[plot]
  type = Plot
  sources = 'fgr1 fgr2'
  file = two_lines.png
  x_label = Time
  y_label = 'Fission gas release'
[]
