[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 1
  []
[]

[AuxVariables]
  [u]
    family = LAGRANGE
    order = FIRST
  []
[]

[UserObjects]
  [source_solution]
    type = SolutionUserObject
    mesh = solution_aux_weighting_source_out.e
    system_variables = source_value
    timestep = LATEST
  []
[]

[AuxKernels]
  [reader]
    type = SolutionAux
    solution = source_solution
    variable = u
    from_variable = source_value
    direct = false
    execute_on = INITIAL
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
