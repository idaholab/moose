[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
  []
[]

[Variables]
  [u_main]
    type = MooseVariableFVReal
  []
[]

[AuxVariables]
  [transferred]
    type = MooseLinearVariableFVReal
  []
[]

[Transfers]
  [copy]
    type = MultiAppCopyTransfer
    from_multi_app = linear_sub
    source_variable = u
    variable = transferred
  []
[]

[MultiApps]
  [linear_sub]
    type = FullSolveMultiApp
    input_files = 'linear_sub.i'
  []
[]

[FVKernels]
  [diff]
    type = FVDiffusion
    variable = u_main
    coeff = 2
  []
[]

[FVBCs]
  [left]
    type = FVDirichletBC
    variable = u_main
    boundary = left
    value = 0
  []

  [./right]
    type = FVDirichletBC
    variable = u_main
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Outputs]
  exodus = true
[]
