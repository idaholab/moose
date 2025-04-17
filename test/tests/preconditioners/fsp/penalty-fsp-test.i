[Mesh]
  [square]
    type = GeneratedMeshGenerator
    nx = 10
    ny = 10
    dim = 2
  []
  second_order = true
[]

[Variables]
  [u]
    order = SECOND
  []
  [v]
    order = SECOND
  []
[]

[Kernels]
  [diff_u]
    type = Diffusion
    variable = u
  []
  [conv_v]
    type = CoupledForce
    variable = v
    v = u
  []
  [diff_v]
    type = Diffusion
    variable = v
  []
[]

[BCs]
  [left_u]
    type = PenaltyDirichletBC
    penalty = 1e6
    variable = u
    boundary = 3
    value = 0
  []
  [right_u]
    type = PenaltyDirichletBC
    penalty = 1e6
    variable = u
    boundary = 1
    value = 100
  []
  [left_v]
    type = PenaltyDirichletBC
    penalty = 1e6
    variable = v
    boundary = 3
    value = 0
  []
  [right_v]
    type = PenaltyDirichletBC
    penalty = 1e6
    variable = v
    boundary = 1
    value = 0
  []
[]

[Executioner]
  type = Steady
[]

[Preconditioning]
  [FSP]
    type = FSP
    topsplit = 'uv'
    [uv]
      splitting = 'u v'
      splitting_type = additive
    []
    [u]
      vars = 'u'
      petsc_options_iname = '-pc_type -ksp_type'
      petsc_options_value = '     hypre preonly'
    []
    [v]
      vars = 'v'
      petsc_options_iname = '-pc_type -ksp_type'
      petsc_options_value = '     lu  preonly'
    []
  []
[]

[Outputs]
  exodus = true
[]
