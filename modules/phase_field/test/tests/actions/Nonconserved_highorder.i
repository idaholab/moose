#
# Test the parsed function free enery Allen-Cahn Bulk kernel
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
  xmax = 40
  ymax = 40
  elem_type = QUAD
  second_order = true
[]

[Modules]
  [./PhaseField]
    [./Nonconserved]
      [./eta]
        family = LAGRANGE
        order = SECOND
        free_energy = F
        kappa = 2.0
        mobility = 1.0
        variable_mobility = false
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
  perf_graph = true
[]
