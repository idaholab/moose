[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 4
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
    type = FVFunctorDirichletBC
    variable = u
    boundary = left
    functor = bc_value
  []
  [right]
    type = FVDirichletBC
    variable = u
    boundary = right
    value = 0
  []
[]

[Materials]
  [bc_value]
    type = GenericFunctorMaterial
    prop_names = bc_value
    prop_values = 10
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
