[Mesh]
  [./Generation]
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 2
    ny = 2
    elem_type = QUAD4
  [../]
[]

[Executioner]
  type = LooseCoupling
  input_files = 'problem1.i problem2.i'
  solve_order = 'problem1.i problem2.i'
  start_time = 0
  end_time = 1
  dt = 0.1
[]

[Output]
  file_base = out_same
  output_initial = true
  interval = 1
  exodus = true
  print_linear_residuals = true
[]
