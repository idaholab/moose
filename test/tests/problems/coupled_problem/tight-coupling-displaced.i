[CoupledProblems]
  [./thermal]
    input_file = therm.i
    [./disp_x]
      from = mechanics
      var_name = disp_x
    [../]
    [./disp_y]
      from = mechanics
      var_name = disp_y
    [../]
  [../]

  [./mechanics]
    input_file = mech.i
    [./temp]
      from = thermal
      var_name = temp
    [../]
  [../]
[]

[Executioner]
  type = CoupledTransientExecutioner
  num_steps = 3
[]
