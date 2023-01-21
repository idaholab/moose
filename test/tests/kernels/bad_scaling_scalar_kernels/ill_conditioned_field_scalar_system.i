[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
[]

[Variables]
  [./u]
  [../]
  [v]
    family = SCALAR
    initial_condition = 1
  []
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [scalar]
    type = ScalarLagrangeMultiplier
    variable = u
    lambda = v
  []
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[ScalarKernels]
  [reaction]
    type = ParsedODEKernel
    expression = '10^20 * v'
    variable = v
  []
  [time]
    type = ODETimeDerivative
    variable = v
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
  dtmin = 1
  solve_type = NEWTON
  petsc_options = '-pc_svd_monitor -ksp_view_pmat -snes_converged_reason -ksp_converged_reason'
  petsc_options_iname = '-pc_type -snes_stol'
  petsc_options_value = 'svd      0'
[]

[Outputs]
  exodus = true
[]
