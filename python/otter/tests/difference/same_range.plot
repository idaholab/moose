[const1]
  type = Constant
  x_values = '0 1 1.8 2.3 4.1 5.4 5.9 7.1 8.2 9.7 10'
  y_values = '25 16 10.24 7.29 0.81 0.16 0.81 4.41 10.24 22.09 25'
[]

[const2]
  type = Constant
  x_values = '0.0 0.3 1.8 2.9 4.1 4.6 5.9 7.7 8.2 9.0 10.0'
  y_values = '25.0 22.09 10.24 4.41 0.81 0.16 0.81 7.29 10.24 16.0 25.0'
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
  file = same_range.csv
[]
