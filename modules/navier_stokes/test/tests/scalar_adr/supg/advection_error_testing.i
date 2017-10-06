velocity=1

[GlobalParams]
  u = ${velocity}
  p = 0
  tau_type = mod
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 4
  xmax = 1
  elem_type = EDGE2
[]

[Variables]
  [./u]
    family = LAGRANGE
    order = FIRST
  [../]
[]

[Kernels]
  [./adv]
    type = Advection
    variable = u
  [../]
  [./frc]
    type = BodyForce
    variable = u
    function = 'ffn'
  [../]
  [./adv_supg]
    type = AdvectionSUPG
    variable = u
  [../]
  [./body_force_supg]
    type = BodyForceSUPG
    variable = u
    function = 'ffn'
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
[]

[Materials]
  [./mat]
    type = GenericConstantMaterial
    prop_names = 'mu rho'
    prop_values = '0 1'
  [../]
[]

[Functions]
  [./ffn]
    type = ParsedFunction
    value = '1-x^2'
  [../]
  [./u_func]
    type = ParsedFunction
    value = 'x-x^3/3'
  [../]
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
[]

[Outputs]
  [./exodus]
    type = Exodus
  [../]
  [./csv]
    type = CSV
  [../]
[]

[Postprocessors]
  [./L2u]
    type = ElementL2Error
    variable = u
    function = u_func
    outputs = 'console csv'
    execute_on = 'timestep_end'
  [../]
  [./L2ux]
    type = ElementL2Error
    variable = ux
    function = ffn
    outputs = 'console csv'
    execute_on = 'timestep_end'
  [../]
[]

[AuxVariables]
  [./ux]
    family = MONOMIAL
    order = FIRST
  [../]
[]

[AuxKernels]
  [./ux]
    type = VariableGradientComponent
    component = x
    variable = ux
    gradient_variable = u
  [../]
[]
