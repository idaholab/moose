[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 4
    ny = 2
  []
[]

[Problem]
  kernel_coverage_check = false
  skip_nl_system_check = true
  solve = false
[]

[AuxVariables]
  [heat_source]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [soln]
    type = SolutionAux
    variable = heat_source
    solution = soln
    from_variable = heat_source
    execute_on = 'INITIAL'
  []
[]

[UserObjects]
  [soln]
    type = SolutionUserObject
    mesh = write_exodus_initial_out.e
    system_variables = 'heat_source'
    execute_on = 'INITIAL'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
  execute_on = INITIAL
[]
