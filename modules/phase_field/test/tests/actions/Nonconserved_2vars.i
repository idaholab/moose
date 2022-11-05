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
      [./eta1]
        free_energy = F
        kappa = 2.0
        mobility = 1.0
        variable_mobility = false
        coupled_variables = 'eta2'
      [../]
      [./eta2]
        free_energy = F
        kappa = 2.0
        mobility = 1.0
        variable_mobility = false
        coupled_variables = 'eta1'
      [../]
    [../]
  [../]
[]

[ICs]
  [./eta1_IC]
    type = SmoothCircleIC
    variable = eta1
    x1 = 20.0
    y1 = 20.0
    radius = 12.0
    invalue = 1.0
    outvalue = 0.0
    int_width = 3.0
  [../]
  [./eta2_IC]
    type = SmoothCircleIC
    variable = eta2
    x1 = 20.0
    y1 = 20.0
    radius = 12.0
    invalue = 0.0
    outvalue = 1.0
    int_width = 3.0
  [../]
[]

[Materials]
  [./free_energy]
    type = DerivativeParsedMaterial
    property_name = F
    coupled_variables = 'eta1 eta2'
    expression = '2.5 * (eta1^4/4 - eta1^2/2 + eta2^4/4 - eta2^2/2 + 3/2 * eta1^2 * eta2^2) + 1/4'
    derivative_order = 2
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
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

  num_steps = 8
  dt = 1.0
[]

[Outputs]
  exodus = true
[]
