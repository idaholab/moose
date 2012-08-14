[Problems]
  [./pr1]
    input_file = problem1.i
  [../]
  [./pr2]
    input_file = problem2.i
    [./u]
      from = pr1
      var_name = u
    [../]
  [../]
[]

[Executioner]
  type = SteadyTransientExecutioner
[]
