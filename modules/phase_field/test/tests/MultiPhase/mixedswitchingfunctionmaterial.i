# This is a test of the MixedSwitchingfunctionmaterial
# Several mixed type of switching function with ajustable weight parameter

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
  xmin = 0
  xmax = 20
  ymin = 0
  ymax = 20
  elem_type = QUAD4
[]

[Variables]
  [./eta]
  [../]
[]

[ICs]
  [./IC_eta]
    type = SmoothCircleIC
    variable = eta
    x1 = 10
    y1 = 10
    radius = 5
    invalue = 1
    outvalue = 0
    int_width = 1
  [../]
[]

[Kernels]
  [./eta_bulk]
    type = AllenCahn
    variable = eta
    f_name = F
  [../]
  [./eta_interface]
    type = ACInterface
    variable = eta
    kappa_name = kappa_eta
  [../]

  [./detadt]
    type = TimeDerivative
    variable = eta
  [../]
[]

[Materials]
  [./consts]
    type = GenericConstantMaterial
    prop_names  = 'L kappa_eta'
    prop_values = '1.0 1.0'
  [../]

  [./switching]
    type = MixedSwitchingFunctionMaterial
    function_name = h
    eta = eta
    h_order = MIX234
    weight = 1.0
  [../]

  [./barrier]
    type = BarrierFunctionMaterial
    eta = eta
    g_order = SIMPLE
  [../]

# Total free energy: F = Fa*(1-h) + Fb*h
  [./free_energy]
    type = DerivativeTwoPhaseMaterial
    property_name = F
    fa_name = '0'
    fb_name = '-1'
    eta = eta
    W = 3.1
    derivative_order = 2
    outputs = exodus
  [../]
[]

[BCs]
  [./Periodic]
    [./all]
      auto_direction = 'x y'
    [../]
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = 'PJFNK'

  l_max_its = 30
  nl_max_its = 10
  l_tol = 1.0e-4
  nl_rel_tol = 1.0e-10
  nl_abs_tol = 1.0e-12
  start_time = 0.0
  num_steps = 2

  [./TimeStepper]
    type = IterationAdaptiveDT
    optimal_iterations = 9
    iteration_window = 2
    growth_factor = 1.1
    cutback_factor = 0.75
    dt = 0.3
  [../]
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
