[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [v][]
  [w][]
[]

[Kernels]
  [diff_v]
    type = ADMatDiffusion
    variable = v
    diffusivity = 1e-20
  []
  [diff_w]
    type = MatDiffusion
    variable = w
    diffusivity = 1e-40
  []
[]

[BCs]
  [left_v]
    type = DirichletBC
    variable = v
    boundary = left
    value = 0
  []
  [right_v]
    type = DirichletBC
    variable = v
    boundary = right
    value = 1
  []
  [left_w]
    type = DirichletBC
    variable = w
    boundary = left
    value = 0
  []
  [right_w]
    type = DirichletBC
    variable = w
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = 'Newton'
  petsc_options = '-pc_svd_monitor'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'svd'
  automatic_scaling = true
  verbose = true
[]

[Outputs]
  exodus = true
[]
