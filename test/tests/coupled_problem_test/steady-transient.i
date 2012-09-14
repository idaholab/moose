[CoupledProblems]
  [./steady]
    input_file = steady.i
  [../]
  [./transient]
    input_file = transient.i
    [./u]
      from = steady
      var_name = u
    [../]
  [../]
[]

[Executioner]
  type = SteadyTransientExecutioner
[]
