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
    variable_L = false
  [../]
[]

[Materials]
  [./consts]
    type = GenericConstantMaterial
    prop_names  = 'L'
    prop_values = '1'
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
  dt = 0.5
[]

[Outputs]
  exodus = true
[]
