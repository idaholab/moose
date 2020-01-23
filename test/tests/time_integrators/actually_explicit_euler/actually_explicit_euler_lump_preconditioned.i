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
  [./time]
    type = TimeDerivative
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    preset = false
    boundary = 'left'
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    preset = false
    boundary = 'right'
    value = 1
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 10
  dt = 0.0001
  l_tol = 1e-12

  [./TimeIntegrator]
    type = ActuallyExplicitEuler
    solve_type = lump_preconditioned
  [../]
[]

[Outputs]
  exodus = true
[]
