[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 10
  ny = 10
  nz = 10
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]

  [./v]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Functions]
  [./forcing_v]
    type = ParsedFunction
    expression = 'x * y * z'
  [../]
[]

[Kernels]
  [./diffusion_u]
    type = Diffusion
    variable = u
  [../]
  [./time_u]
    type = TimeDerivative
    variable = u
  [../]

  [./diffusion_v]
    type = Diffusion
    variable = v
  [../]
  [./forcing_v]
    type = BodyForce
    variable = v
    function = forcing_v
  [../]
  [./time_v]
    type = TimeDerivative
    variable = v
  [../]
[]

[BCs]
  [./bottom]
    type = DirichletBC
    variable = 'u'
    boundary = 'bottom'
    value = 1
  [../]

  [./top]
    type = DirichletBC
    variable = 'u'
    boundary = 'top'
    value = 0
  [../]
[]

[VectorPostprocessors]
  [./difference]
    type = ElementVariablesDifferenceMax
    compare_a = u
    compare_b = v
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 2
  dt = 1
  solve_type = PJFNK
[]

[Outputs]
  execute_on = 'initial timestep_end'
  csv = true
[]
