#
# Test the parsed function free enery Allen-Cahn Bulk kernel
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 40
  ny = 40
  xmax = 40
  ymax = 40
  elem_type = QUAD
[]

[Modules]
  [./PhaseField]
    [./Nonconserved]
      [./eta]
        free_energy = F
        kappa = 2.0
        mobility = variable_L
      [../]
    [../]
  [../]
[]

[ICs]
  [./InitialCondition]
    type = SmoothCircleIC
    variable = eta
    x1 = 20.0
    y1 = 20.0
    radius = 6.0
    invalue = 0.9
    outvalue = 0.1
    int_width = 3.0
  [../]
[]

[Materials]
  [./mobility]
    type = DerivativeParsedMaterial
    property_name = variable_L
    coupled_variables = 'eta'
    expression = '0.5 * eta + 1.5 * (1 - eta)'
    derivative_order = 1
    outputs = exodus
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

  l_max_its = 15
  l_tol = 1.0e-4

  nl_max_its = 10
  nl_rel_tol = 1.0e-11

  num_steps = 10
  dt = 1.0
[]

[Outputs]
  exodus = true
[]
