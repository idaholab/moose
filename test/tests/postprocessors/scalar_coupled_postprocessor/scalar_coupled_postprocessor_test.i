[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
  xmax = 1
  ymax = 1
  elem_type = QUAD4
[]

[Variables]
  [./u]
    initial_condition = 1
  [../]
  [./scalar_variable]
    family = SCALAR
    order = FIRST
    initial_condition = 2
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[ScalarKernels]
  [./td1]
    type = ODETimeDerivative
    variable = scalar_variable
  [../]
[]

[BCs]
  [./leftDirichlet]
      type = DirichletBC
      variable = u
      boundary = 'left'
      value = 1
  [../]
  [./rightDirichlet]
      type = DirichletBC
      variable = u
      boundary = 'right'
      value = 0
  [../]
[]

[Postprocessors]
  [./totalFlux]
    type = ScalarCoupledPostprocessor
    variable = u
    coupled_scalar = scalar_variable
    boundary = left
  [../]
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 1
  solve_type = JFNK
  l_max_its = 30
  l_tol = 1e-6
  nl_max_its = 20
  nl_rel_tol = 1e-5
[]

[Outputs]
  csv = true
[]
