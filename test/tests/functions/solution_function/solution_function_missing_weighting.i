[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 1
  []
[]

[UserObjects]
  [source_solution]
    type = SolutionUserObject
    mesh = solution_function_weighting_source_out.e
    system_variables = source_value
    timestep = LATEST
  []
[]

[Functions]
  [reader]
    type = SolutionFunction
    solution = source_solution
    from_variable = source_value
  []
[]

[Problem]
  kernel_coverage_check = false
  skip_nl_system_check = true
  solve = false
[]

[Executioner]
  type = Steady
[]
