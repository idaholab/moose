[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
  second_order = false
  parallel_type = REPLICATED
[]

[Problem]
  solve=false
[]

[AuxVariables]
  [restart_source]
    order = FIRST
    family = LAGRANGE
  []
[]

[UserObjects]
  [restart_soln]
    type = SolutionUserObject
    mesh = main_out_forward0.e
    system_variables = source
  []
[]

[AuxKernels]
  [restart_source]
    type = SolutionAux
    variable = restart_source
    solution = restart_soln
  []
[]

[BCs]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
