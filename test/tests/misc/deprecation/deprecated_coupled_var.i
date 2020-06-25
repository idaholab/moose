[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
  [../]
  [v][]
[]

[Kernels]
  active = 'diff_u coupled_u diff_v deprecated_coupled_v'
  [./diff_u]
    type = Diffusion
    variable = u
  [../]
  [coupled_u]
    type = DeprecatedCoupledVarKernel
    variable = u
    source = v
  []
  [diff_v]
    type = Diffusion
    variable = v
  []
  [deprecated_coupled_v]
    type = DeprecatedCoupledVarKernel
    variable = v
    stupid_name = u
  []
  [blessed_coupled_v]
    type = DeprecatedCoupledVarKernel
    variable = v
    source = u
  []
[]

[BCs]
  [./left_u]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right_u]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
  [./left_v]
    type = DirichletBC
    variable = v
    boundary = left
    value = 1
  [../]
  [./right_v]
    type = DirichletBC
    variable = v
    boundary = right
    value = 0
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Outputs]
  exodus = true
[]
