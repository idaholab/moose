[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 10
  nz = 0
  xmin = -10
  xmax = 10
  ymin = -5
  ymax = 5
  elem_type = QUAD4
[]

[AuxVariables]
  [./eta1]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = SmoothCircleIC
      x1 = -3.5
      y1 =  0.0
      radius = 4.0
      invalue = 0.9
      outvalue = 0.1
      int_width = 2.0
    [../]
  [../]
[]

[Variables]
  [./eta2]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = SmoothCircleIC
      x1 =  3.5
      y1 =  0.0
      radius = 4.0
      invalue = 0.9
      outvalue = 0.1
      int_width = 2.0
    [../]
  [../]
  [./eta3]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = SpecifiedSmoothCircleIC
      x_positions = '-4.0 4.0'
      y_positions = ' 0.0 0.0'
      z_positions = ' 0.0 0.0'
      radii = '4.0 4.0'
      invalue = 0.1
      outvalue = 0.9
      int_width = 2.0
    [../]
  [../]

  [./lambda]
    order = FIRST
    family = LAGRANGE
    initial_condition = 1.0
  [../]
[]

[Kernels]
  [./deta2dt]
    type = TimeDerivative
    variable = eta2
  [../]
  [./ACBulk2]
    type = AllenCahn
    variable = eta2
    coupled_variables = 'eta1 eta3'
    mob_name = L2
    f_name = F
  [../]
  [./ACInterface2]
    type = ACMultiInterface
    variable = eta2
    etas = 'eta1 eta2 eta3'
    mob_name = L2
    kappa_names = 'kappa21 kappa22 kappa23'
  [../]
  [./lagrange2]
    type = SwitchingFunctionConstraintEta
    variable = eta2
    h_name   = h2
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
    mob_name = L3
    f_name = F
  [../]
  [./ACInterface3]
    type = ACMultiInterface
    variable = eta3
    etas = 'eta1 eta2 eta3'
    mob_name = L3
    kappa_names = 'kappa31 kappa32 kappa33'
  [../]
  [./lagrange3]
    type = SwitchingFunctionConstraintEta
    variable = eta3
    h_name   = h3
    lambda = lambda
  [../]

  [./lagrange]
    type = SwitchingFunctionConstraintLagrange
    variable = lambda
    etas    = 'eta1 eta2 eta3'
    h_names = 'h1   h2   h3'
    epsilon = 0
  [../]
[]

[BCs]
  [./Periodic]
    [./All]
      auto_direction = 'x y'
    [../]
  [../]
[]

[Materials]
  [./consts]
    type = GenericConstantMaterial
    prop_names  = 'Fx  L1 L2 L3  kappa11 kappa12 kappa13 kappa21 kappa22 kappa23 kappa31 kappa32 kappa33'
    prop_values = '0   1  1  1   1       1       1       1       1       1       1       1       1      '
  [../]

  [./switching1]
    type = SwitchingFunctionMaterial
    function_name = h1
    eta = eta1
    h_order = SIMPLE
  [../]
  [./switching2]
    type = SwitchingFunctionMaterial
    function_name = h2
    eta = eta2
    h_order = SIMPLE
  [../]
  [./switching3]
    type = SwitchingFunctionMaterial
    function_name = h3
    eta = eta3
    h_order = SIMPLE
  [../]

  [./barrier]
    type = MultiBarrierFunctionMaterial
    etas = 'eta1 eta2 eta3'
  [../]

  [./free_energy]
    type = DerivativeMultiPhaseMaterial
    property_name = F
    # we use a constant free energy (GeneriConstantmaterial property Fx)
    fi_names = 'Fx  Fx  Fx'
    hi_names = 'h1  h2  h3'
    etas     = 'eta1 eta2 eta3'
    # the free energy is given by the MultiBarrierFunctionMaterial only
    W = 1
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

  solve_type = 'PJFNK'
  #petsc_options = '-snes_ksp -snes_ksp_ew'
  #petsc_options = '-ksp_monitor_snes_lg-snes_ksp_ew'
  #petsc_options_iname = '-ksp_gmres_restart'
  #petsc_options_value = '1000              '

  l_max_its = 15
  l_tol = 1.0e-6

  nl_max_its = 50
  nl_rel_tol = 1.0e-8
  nl_abs_tol = 1.0e-10

  start_time = 0.0
  num_steps = 2
  dt = 0.2
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
