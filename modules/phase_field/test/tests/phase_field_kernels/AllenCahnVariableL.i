#
# Test the parsed function free enery Allen-Cahn Bulk kernel
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmax = 12
  ymax = 12
  elem_type = QUAD4
[]

[AuxVariables]
  [./chi]
    [./InitialCondition]
      type = FunctionIC
      function = 'x/24+0.5'
    [../]
  [../]
[]

[Variables]
  [./eta]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = SmoothCircleIC
      x1 = 0.0
      y1 = 0.0
      radius = 6.0
      invalue = 0.9
      outvalue = 0.1
      int_width = 3.0
    [../]
  [../]
[]

[Kernels]
  [./detadt]
    type = TimeDerivative
    variable = eta
  [../]

  [./ACBulk]
    type = AllenCahn
    variable = eta
    f_name = F
  [../]

  [./ACInterface]
    type = ACInterface
    variable = eta
    kappa_name = 1
    variable_L = true
    coupled_variables = chi
  [../]
[]

[Materials]
  [./L]
    type = DerivativeParsedMaterial
    property_name = L
    coupled_variables = 'eta chi'
    expression = '0.1 * eta^2 + chi^2'
    derivative_order = 2
  [../]

  [./free_energy]
    type = DerivativeParsedMaterial
    property_name = F
    coupled_variables = 'eta'
    expression = '2 * eta^2 * (1-eta)^2 - 0.2*eta'
    derivative_order = 2
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'
  solve_type = 'NEWTON'
  num_steps = 2
  dt = 1
[]

[Outputs]
  exodus = true
[]
