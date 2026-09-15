[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 30
  ny = 30
  xmin = 0
  xmax = 30
  ymin = 0
  ymax = 30
[]

[Variables]
  [./eta1]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = SmoothCircleIC
      x1 = 15.0
      y1 = 22.0
      radius = 9.0
      invalue = 1.0
      outvalue = 0.0
      int_width = 3.0
    [../]
  [../]
  [./eta2]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = SmoothCircleIC
      x1 = 8.94
      y1 = 11.5
      radius = 9.0
      invalue = 1.0
      outvalue = 0.0
      int_width = 3.0
    [../]
  [../]
  [./eta3]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = SmoothCircleIC
      x1 = 21.06
      y1 = 11.5
      radius = 9.0
      invalue = 1.0
      outvalue = 0.0
      int_width = 3.0
    [../]
  [../]
  [./lambda]
    order = FIRST
    family = LAGRANGE
    initial_condition = 1.0
  [../]
[]

[AuxVariables]
  [./eta_sum]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxKernels]
  [./eta_sum_aux]
    type = ParsedAux
    variable = eta_sum
    coupled_variables = 'eta1 eta2 eta3'
    expression = 'eta1 + eta2 + eta3'
    execute_on = 'TIMESTEP_END'
  [../]
[]

[Kernels]
  [./deta1dt]
    type = TimeDerivative
    variable = eta1
  [../]
  [./ACBulk1]
    type = AllenCahn
    variable = eta1
    coupled_variables = 'eta2 eta3'
    f_name = F
  [../]
  [./ACInterface1]
    type = ACInterface
    variable = eta1
    kappa_name = kappa_op
  [../]
  [./lagrange1]
    type = SwitchingFunctionConstraintEta
    variable = eta1
    h_name = hp1
    lambda = lambda
  [../]

  [./deta2dt]
    type = TimeDerivative
    variable = eta2
  [../]
  [./ACBulk2]
    type = AllenCahn
    variable = eta2
    coupled_variables = 'eta1 eta3'
    f_name = F
  [../]
  [./ACInterface2]
    type = ACInterface
    variable = eta2
    kappa_name = kappa_op
  [../]
  [./lagrange2]
    type = SwitchingFunctionConstraintEta
    variable = eta2
    h_name = hp2
    lambda = lambda
  [../]

  [./deta3dt]
    type = TimeDerivative
    variable = eta3
  [../]
  [./ACBulk3]
    type = AllenCahn
    variable = eta3
    coupled_variables = 'eta1 eta2'
    f_name = F
  [../]
  [./ACInterface3]
    type = ACInterface
    variable = eta3
    kappa_name = kappa_op
  [../]
  [./lagrange3]
    type = SwitchingFunctionConstraintEta
    variable = eta3
    h_name = hp3
    lambda = lambda
  [../]

  [./lagrange]
    type = SwitchingFunctionConstraintLagrange
    variable = lambda
    etas = 'eta1 eta2 eta3'
    h_names = 'hp1 hp2 hp3'
    epsilon = 0
  [../]
[]

[Materials]
  [./consts]
    type = GenericConstantMaterial
    prop_names = 'L kappa_op'
    prop_values = '1  1'
  [../]

  [./g1]
    type = SwitchingFunctionMultiPhaseKundinMaterial
    h_name = g1
    eta_i = eta1
    all_etas = 'eta1 eta2 eta3'
  [../]
  [./g2]
    type = SwitchingFunctionMultiPhaseKundinMaterial
    h_name = g2
    eta_i = eta2
    all_etas = 'eta1 eta2 eta3'
  [../]
  [./g3]
    type = SwitchingFunctionMultiPhaseKundinMaterial
    h_name = g3
    eta_i = eta3
    all_etas = 'eta1 eta2 eta3'
  [../]

  [./barrier]
    type = MultiBarrierFunctionMaterial
    etas = 'eta1 eta2 eta3'
    function_name = barrier
  [../]

  [./obstacle]
    type = TripleJunctionBarrierFunctionMaterial
    etas = 'eta1 eta2 eta3'
    h = 5
  [../]

  [./hp1]
    type = DerivativeParsedMaterial
    property_name = hp1
    coupled_variables = 'eta1 eta2 eta3'
    expression = 'eta1'
    derivative_order = 2
  [../]
  [./hp2]
    type = DerivativeParsedMaterial
    property_name = hp2
    coupled_variables = 'eta1 eta2 eta3'
    expression = 'eta2'
    derivative_order = 2
  [../]
  [./hp3]
    type = DerivativeParsedMaterial
    property_name = hp3
    coupled_variables = 'eta1 eta2 eta3'
    expression = 'eta3'
    derivative_order = 2
  [../]

  [./free_energy]
    type = DerivativeSumMaterial
    coupled_variables = 'eta1 eta2 eta3'
    sum_materials = 'g1   g2     g3     barrier f_obs'
    prefactor     = '0.0  -0.02  -0.04  1       1'
  [../]
[]

[Postprocessors]
  [./avg_eta_sum]
    type = ElementAverageValue
    variable = eta_sum
  [../]
  [./max_eta_sum]
    type = ElementExtremeValue
    variable = eta_sum
    value_type = max
  [../]
  [./min_eta_sum]
    type = ElementExtremeValue
    variable = eta_sum
    value_type = min
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

  solve_type = 'PJFNK'

  l_max_its = 15
  l_tol = 1.0e-6

  nl_max_its = 50
  nl_rel_tol = 1.0e-8
  nl_abs_tol = 1.0e-10

  start_time = 0.0
  num_steps = 3
  dt = 0.05
  dtmin = 0.05
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
  csv = true
[]
