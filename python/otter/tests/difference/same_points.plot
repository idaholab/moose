[const1]
  type = Constant
  x_values = '0 1 1.8 2.3 4.1 5.4 5.9 7.1 8.2 9.7 10'
  y_values = '25 16 10.24 7.29 0.81 0.16 0.81 4.41 10.24 22.09 25'
[]

[const2]
  type = Constant
  x_values = '0 1 1.8 2.3 4.1 5.4 5.9 7.1 8.2 9.7 10'
  y_values = '20.25 12.25 7.29 4.84 0.16 0.81 1.96 6.76 13.69 27.04 30.25'
[]

[diff]
  type = Difference
  source1 = const1
  source2 = const2
[]

[csv]
  type = WriteCSV
  sources = 'diff'
  x_name = x
  file = same_points.csv
[]
