[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
[]

[Variables]
  [u]
    family = MONOMIAL
    order = CONSTANT
    fv = true
  []
[]

[FVKernels]
  [diff]
    type = FVDiffusion
    variable = u
    coeff = 1
  []
[]

[FVBCs]
  [left]
    type = FVPostprocessorDirichletBC
    variable = u
    boundary = left
    postprocessor = bc_val
  []
  [right]
    type = FVDirichletBC
    variable = u
    boundary = right
    value = 0
  []
[]

[Postprocessors]
  [bc_val]
    type = Receiver
    default = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = 'Newton'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  exodus = true
[]
