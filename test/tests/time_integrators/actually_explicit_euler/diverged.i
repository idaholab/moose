[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
  [../]
  [./nan]
    type = NanKernel
    variable = u
    timestep_to_nan = 4
  [../]
  [./time]
    type = TimeDerivative
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 'left'
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 'right'
    value = 1
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 10
  dt = 0.001
  l_tol = 1e-12
  dtmin = 1e-8

  [./TimeIntegrator]
    type = ActuallyExplicitEuler
    solve_type = lump_preconditioned
  [../]
[]

[Outputs]
  exodus = false
[]
