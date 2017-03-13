# The most important part of this tests is the fact we have the p000 postprocessor
# active. That triggers computeUserObject loop inbetween computing the initial
# stateful material property values.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
[]

[Variables]
  [./u1]
    initial_condition = 1
  [../]
[]

[AuxVariables]
  [./aux1]
    initial_condition = 1234
  [../]
[]

[Kernels]
  [./td1]
    type = TimeDerivative
    variable = u1
  [../]
  [./diff1]
    type = CoefDiffusion
    variable = u1
    coef = 0.1
  [../]
[]

[Materials]
  [./coupled_mat]
    type = AuxCoupledMaterial
    variable = 'aux1'
  [../]
[]

[BCs]
  [./left1]
    type = DirichletBC
    variable = u1
    boundary = left
    value = 0
  [../]
  [./right1]
    type = DirichletBC
    variable = u1
    boundary = right
    value = 1
  [../]
[]

[Postprocessors]
  [./p000]
    type = PointValue
    variable = u1
    point = '0 0 0'
    execute_on = 'initial timestep_end'
  [../]
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 1
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]
