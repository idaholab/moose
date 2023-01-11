velocity=1

[GlobalParams]
  u = ${velocity}
  pressure = 0
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
  [./c]
    family = LAGRANGE
    order = FIRST
  [../]
[]

[Kernels]
  [./adv]
    type = Advection
    variable = c
    forcing_func = 'ffn'
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = c
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
    expression = '1-x^2'
  [../]
  [./c_func]
    type = ParsedFunction
    expression = 'x-x^3/3'
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
  [./L2c]
    type = ElementL2Error
    variable = c
    function = c_func
    outputs = 'console'    execute_on = 'timestep_end'
  [../]
  [./L2cx]
    type = ElementL2Error
    variable = cx
    function = ffn
    outputs = 'console'    execute_on = 'timestep_end'
  [../]
[]

[AuxVariables]
  [./cx]
    family = MONOMIAL
    order = FIRST
  [../]
[]

[AuxKernels]
  [./cx]
    type = VariableGradientComponent
    component = x
    variable = cx
    gradient_variable = c
  [../]
[]
