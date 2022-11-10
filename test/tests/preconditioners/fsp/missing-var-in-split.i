[Mesh]
  [square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  []
[]

[Variables]
  [u][]
  [v][]
  [w][]
[]

[Kernels]
  [diff_u]
    type = Diffusion
    variable = u
  []
  [diff_v]
    type = Diffusion
    variable = v
  []
  [diff_w]
    type = Diffusion
    variable = w
  []
[]

[BCs]
  [left_u]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  []
  [right_u]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 100
  []
  [left_v]
    type = DirichletBC
    variable = v
    boundary = 3
    value = 0
  []
  [right_v]
    type = DirichletBC
    variable = v
    boundary = 1
    value = 0
  []
  [left_w]
    type = DirichletBC
    variable = w
    boundary = 3
    value = 0
  []
  [right_w]
    type = DirichletBC
    variable = w
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
      splitting_type  = additive
    []
    [u]
      vars = 'u'
      petsc_options_iname = '-pc_type -ksp_type'
      petsc_options_value = '     hypre preonly'
    []
    [v]
      vars = 'v'
      petsc_options_iname = '-pc_type -ksp_type'
      petsc_options_value = '     hypre  preonly'
    []
  []
[]
