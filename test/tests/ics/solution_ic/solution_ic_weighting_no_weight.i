[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 1
  []
[]

[Variables]
  [u]
    family = LAGRANGE
    order = FIRST
  []
[]

[UserObjects]
  [source_solution]
    type = SolutionUserObject
    mesh = solution_ic_weighting_source_out.e
    system_variables = source_value
    timestep = LATEST
  []
[]

[ICs]
  [reader]
    type = SolutionIC
    solution_uo = source_solution
    variable = u
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