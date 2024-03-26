#
# This test is for the damped nested solve of 3-phase KKS model, and uses log-based free energies.
# The split-form of the Cahn-Hilliard equation instead of the Fick's diffusion equation is solved

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
  nz = 0
  xmin = 0
  xmax = 40
  ymin = 0
  ymax = 40
  zmin = 0
  zmax = 0
  elem_type = QUAD4
[]

[BCs]
  [Periodic]
    [all]
      auto_direction = 'x y'
    []
  []
[]

[AuxVariables]
  [Energy]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Variables]
  # concentration
  [c]
    order = FIRST
    family = LAGRANGE
  []

  # order parameter 1
  [eta1]
    order = FIRST
    family = LAGRANGE
  []

  # order parameter 2
  [eta2]
    order = FIRST
    family = LAGRANGE
  []

  # order parameter 3
  [eta3]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.0
  []

  # chemical potential
  [mu]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.0
  []

  # Lagrange multiplier
  [lambda]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.0
  []
[]

[ICs]
  [eta1]
    variable = eta1
    type = SmoothCircleIC
    x1 = 20.0
    y1 = 20.0
    radius = 10
    invalue = 0.9
    outvalue = 0.1
    int_width = 4
  []
  [eta2]
    variable = eta2
    type = SmoothCircleIC
    x1 = 20.0
    y1 = 20.0
    radius = 10
    invalue = 0.1
    outvalue = 0.9
    int_width = 4
  []
  [c]
    variable = c
    type = SmoothCircleIC
    x1 = 20.0
    y1 = 20.0
    radius = 10
    invalue = 0.2
    outvalue = 0.5
    int_width = 2
  []
[]

[Materials]
  # simple toy free energies
  [F1]
    type = DerivativeParsedMaterial
    property_name = F1
    expression = 'c1*log(c1/1e-4) + (1-c1)*log((1-c1)/(1-1e-4))'
    material_property_names = 'c1'
    additional_derivative_symbols = 'c1'
    compute = false
  []
  [F2]
    type = DerivativeParsedMaterial
    property_name = F2
    expression = 'c2*log(c2/0.5) + (1-c2)*log((1-c2)/(1-0.5))'
    material_property_names = 'c2'
    additional_derivative_symbols = 'c2'
    compute = false
  []
  [F3]
    type = DerivativeParsedMaterial
    property_name = F3
    expression = 'c3*log(c3/0.9999) + (1-c3)*log((1-c3)/(1-0.9999))'
    material_property_names = 'c3'
    additional_derivative_symbols = 'c3'
    compute = false
  []
  [C]
    type = DerivativeParsedMaterial
    property_name = 'C'
    material_property_names = 'c1 c2 c3'
    expression = '(c1>0)&(c1<1)&(c2>0)&(c2<1)&(c3>0)&(c3<1)'
    compute = false
  []
  [KKSPhaseConcentrationMultiPhaseMaterial]
    type = KKSPhaseConcentrationMultiPhaseMaterial
    global_cs = 'c'
    all_etas = 'eta1 eta2 eta3'
    hj_names = 'h1 h2 h3'
    ci_names = 'c1 c2 c3'
    ci_IC = '0.2 0.5 0.8'
    Fj_names = 'F1 F2 F3'
    min_iterations = 1
    max_iterations = 1000
    absolute_tolerance = 1e-15
    relative_tolerance = 1e-8
    step_size_tolerance = 1e-05
    damped_Newton = true
    conditions = C
    damping_factor = 0.8
  []
  [KKSPhaseConcentrationMultiPhaseDerivatives]
    type = KKSPhaseConcentrationMultiPhaseDerivatives
    global_cs = 'c'
    all_etas = 'eta1 eta2 eta3'
    Fj_names = 'F1 F2 F3'
    hj_names = 'h1 h2 h3'
    ci_names = 'c1 c2 c3'
  []

  # Switching functions for each phase
  # h1(eta1, eta2, eta3)
  [h1]
    type = SwitchingFunction3PhaseMaterial
    eta_i = eta1
    eta_j = eta2
    eta_k = eta3
    property_name = h1
  []
  # h2(eta1, eta2, eta3)
  [h2]
    type = SwitchingFunction3PhaseMaterial
    eta_i = eta2
    eta_j = eta3
    eta_k = eta1
    property_name = h2
  []
  # h3(eta1, eta2, eta3)
  [h3]
    type = SwitchingFunction3PhaseMaterial
    eta_i = eta3
    eta_j = eta1
    eta_k = eta2
    property_name = h3
  []

  # Barrier functions for each phase
  [g1]
    type = BarrierFunctionMaterial
    g_order = SIMPLE
    eta = eta1
    function_name = g1
  []
  [g2]
    type = BarrierFunctionMaterial
    g_order = SIMPLE
    eta = eta2
    function_name = g2
  []
  [g3]
    type = BarrierFunctionMaterial
    g_order = SIMPLE
    eta = eta3
    function_name = g3
  []

  # constant properties
  [constants]
    type = GenericConstantMaterial
    prop_names = 'L   kappa  M'
    prop_values = '0.7 1.0    0.025'
  []
[]

[Kernels]
  [lambda_lagrange]
    type = SwitchingFunctionConstraintLagrange
    variable = lambda
    etas = 'eta1 eta2 eta3'
    h_names = 'h1   h2   h3'
    epsilon = 1e-04
  []
  [eta1_lagrange]
    type = SwitchingFunctionConstraintEta
    variable = eta1
    h_name = h1
    lambda = lambda
    coupled_variables = 'eta2 eta3'
  []
  [eta2_lagrange]
    type = SwitchingFunctionConstraintEta
    variable = eta2
    h_name = h2
    lambda = lambda
    coupled_variables = 'eta1 eta3'
  []
  [eta3_lagrange]
    type = SwitchingFunctionConstraintEta
    variable = eta3
    h_name = h3
    lambda = lambda
    coupled_variables = 'eta1 eta2'
  []

  #Kernels for Cahn-Hilliard equation
  [diff_time]
    type = CoupledTimeDerivative
    variable = mu
    v = c
  []
  [CHBulk]
    type = NestedKKSMultiSplitCHCRes
    variable = c
    all_etas = 'eta1 eta2 eta3'
    global_cs = 'c'
    w = mu
    c1_names = 'c1'
    F1_name = F1
    coupled_variables = 'eta1 eta2 eta3 mu'
  []
  [ckernel]
    type = SplitCHWRes
    variable = mu
    mob_name = M
  []

  # Kernels for Allen-Cahn equation for eta1
  [deta1dt]
    type = TimeDerivative
    variable = eta1
  []
  [ACBulkF1]
    type = NestedKKSMultiACBulkF
    variable = eta1
    global_cs = 'c'
    eta_i = eta1
    all_etas = 'eta1 eta2 eta3'
    ci_names = 'c1 c2 c3'
    hj_names = 'h1 h2 h3'
    Fj_names = 'F1 F2 F3'
    gi_name = g1
    mob_name = L
    wi = 1.0
    coupled_variables = 'c eta2 eta3'
  []
  [ACBulkC1]
    type = NestedKKSMultiACBulkC
    variable = eta1
    global_cs = 'c'
    eta_i = eta1
    all_etas = 'eta1 eta2 eta3'
    ci_names = 'c1 c2 c3'
    hj_names = 'h1 h2 h3'
    Fj_names = 'F1 F2 F3'
    coupled_variables = 'c eta2 eta3'
  []
  [ACInterface1]
    type = ACInterface
    variable = eta1
    kappa_name = kappa
  []

  # Kernels for Allen-Cahn equation for eta2
  [deta2dt]
    type = TimeDerivative
    variable = eta2
  []
  [ACBulkF2]
    type = NestedKKSMultiACBulkF
    variable = eta2
    global_cs = 'c'
    eta_i = eta2
    all_etas = 'eta1 eta2 eta3'
    ci_names = 'c1 c2 c3'
    hj_names = 'h1 h2 h3'
    Fj_names = 'F1 F2 F3'
    gi_name = g2
    mob_name = L
    wi = 1.0
    coupled_variables = 'c eta1 eta3'
  []
  [ACBulkC2]
    type = NestedKKSMultiACBulkC
    variable = eta2
    global_cs = 'c'
    eta_i = eta2
    all_etas = 'eta1 eta2 eta3'
    ci_names = 'c1 c2 c3'
    hj_names = 'h1 h2 h3'
    Fj_names = 'F1 F2 F3'
    coupled_variables = 'c eta1 eta3'
  []
  [ACInterface2]
    type = ACInterface
    variable = eta2
    kappa_name = kappa
  []

  # Kernels for Allen-Cahn equation for eta3
  [deta3dt]
    type = TimeDerivative
    variable = eta3
  []
  [ACBulkF3]
    type = NestedKKSMultiACBulkF
    variable = eta3
    global_cs = 'c'
    eta_i = eta3
    all_etas = 'eta1 eta2 eta3'
    ci_names = 'c1 c2 c3'
    hj_names = 'h1 h2 h3'
    Fj_names = 'F1 F2 F3'
    gi_name = g3
    mob_name = L
    wi = 1.0
    coupled_variables = 'c eta1 eta2'
  []
  [ACBulkC3]
    type = NestedKKSMultiACBulkC
    variable = eta3
    global_cs = 'c'
    eta_i = eta3
    all_etas = 'eta1 eta2 eta3'
    ci_names = 'c1 c2 c3'
    hj_names = 'h1 h2 h3'
    Fj_names = 'F1 F2 F3'
    coupled_variables = 'c eta1 eta2'
  []
  [ACInterface3]
    type = ACInterface
    variable = eta3
    kappa_name = kappa
  []
[]

[AuxKernels]
  [Energy_total]
    type = KKSMultiFreeEnergy
    Fj_names = 'F1 F2 F3'
    hj_names = 'h1 h2 h3'
    gj_names = 'g1 g2 g3'
    variable = Energy
    w = 1
    interfacial_vars = 'eta1  eta2  eta3'
    kappa_names = 'kappa kappa kappa'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -sub_pc_type   -sub_pc_factor_shift_type'
  petsc_options_value = 'asm       ilu            nonzero'
  l_max_its = 30
  nl_max_its = 10
  l_tol = 1.0e-4
  nl_rel_tol = 1.0e-10
  nl_abs_tol = 1.0e-11

  num_steps = 2
  dt = 0.01
[]

[Preconditioning]
  active = 'full'
  [full]
    type = SMP
    full = true
  []
  [mydebug]
    type = FDP
    full = true
  []
[]

[Outputs]
  file_base = kks_example_multiphase_nested_damped
  exodus = true
[]
