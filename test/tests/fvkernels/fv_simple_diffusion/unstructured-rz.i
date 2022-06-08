[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
  elem_type = TRI3
[]

[Variables]
  [v]
    family = MONOMIAL
    order = CONSTANT
    fv = true
  []
[]

[FVKernels]
  [diff]
    type = FVDiffusion
    variable = v
    coeff = coeff
  []
[]

[Materials]
  [diff]
    type = ADGenericFunctorMaterial
    prop_names = 'coeff'
    prop_values = '1'
  []
[]

[FVBCs]
  [right]
    type = FVDirichletBC
    boundary = right
    value = 1
    variable = v
  []
[]


[Problem]
  type = FEProblem
  coord_type = RZ
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  residual_and_jacobian_together = true
[]

[Outputs]
  exodus = true
[]
