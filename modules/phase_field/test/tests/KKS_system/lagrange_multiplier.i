#
# This test ensures that the equilibrium solution using two order parameters with a
# Lagrange multiplier constraint is identical to the dedicated two phase formulation
# in two_phase.i
#

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
  xmax = 5
[]

[AuxVariables]
  [Fglobal]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Variables]
  # concentration
  [c]
    order = FIRST
    family = LAGRANGE
    [InitialCondition]
      type = FunctionIC
      function = x/5
    []
  []

  # order parameter 1
  [eta1]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.5
  []

  # order parameter 2
  [eta2]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.5
  []

  # phase concentration 1
  [c1]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.2
  []

  # phase concentration 2
  [c2]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.5
  []

  # Lagrange multiplier
  [lambda]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.0
  []
[]

[Materials]
  # simple toy free energies
  [f1] # = fd
    type = DerivativeParsedMaterial
    property_name = F1
    coupled_variables = 'c1'
    expression = '(0.9-c1)^2'
  []
  [f2] # = fm
    type = DerivativeParsedMaterial
    property_name = F2
    coupled_variables = 'c2'
    expression = '(0.1-c2)^2'
  []

  # Switching functions for each phase
  [h1_eta]
    type = SwitchingFunctionMaterial
    h_order = HIGH
    eta = eta1
    function_name = h1
  []
  [h2_eta]
    type = SwitchingFunctionMaterial
    h_order = HIGH
    eta = eta2
    function_name = h2
  []

  # Coefficients for diffusion equation
  [Dh1]
    type = DerivativeParsedMaterial
    material_property_names = 'D h1'
    expression = D*h1
    property_name = Dh1
  []
  [Dh2]
    type = DerivativeParsedMaterial
    material_property_names = 'D h2'
    expression = D*h2
    property_name = Dh2
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

  # constant properties
  [constants]
    type = GenericConstantMaterial
    prop_names  = 'D   L   kappa'
    prop_values = '0.7 0.7 0.2'
  []
[]

[Kernels]
  #Kernels for diffusion equation
  [diff_time]
    type = TimeDerivative
    variable = c
  []
  [diff_c1]
    type = MatDiffusion
    variable = c
    diffusivity = Dh1
    v = c1
  []
  [diff_c2]
    type = MatDiffusion
    variable = c
    diffusivity = Dh2
    v = c2
  []

  # Kernels for Allen-Cahn equation for eta1
  [deta1dt]
    type = TimeDerivative
    variable = eta1
  []
  [ACBulkF1]
    type = KKSMultiACBulkF
    variable = eta1
    Fj_names = 'F1 F2 '
    hj_names = 'h1 h2 '
    gi_name = g1
    eta_i = eta1
    wi = 0.2
    coupled_variables = 'c1 c2 eta2'
  []
  [ACBulkC1]
    type = KKSMultiACBulkC
    variable = eta1
    Fj_names = 'F1 F2'
    hj_names = 'h1 h2'
    cj_names = 'c1 c2'
    eta_i = eta1
    coupled_variables = 'eta2'
  []
  [ACInterface1]
    type = ACInterface
    variable = eta1
    kappa_name = kappa
  []
  [multipler1]
    type = MatReaction
    variable = eta1
    v = lambda
    mob_name = L
  []

  # Kernels for the Lagrange multiplier equation
  [mult_lambda]
    type = MatReaction
    variable = lambda
    mob_name = 2
  []
  [mult_ACBulkF_1]
    type = KKSMultiACBulkF
    variable = lambda
    Fj_names = 'F1 F2 '
    hj_names = 'h1 h2 '
    gi_name = g1
    eta_i = eta1
    wi = 0.2
    mob_name = 1
    coupled_variables = 'c1 c2 eta2 '
  []
  [mult_ACBulkC_1]
    type = KKSMultiACBulkC
    variable = lambda
    Fj_names = 'F1 F2'
    hj_names = 'h1 h2'
    cj_names = 'c1 c2'
    eta_i = eta1
    coupled_variables = 'eta2 '
    mob_name = 1
  []
  [mult_CoupledACint_1]
    type = SimpleCoupledACInterface
    variable = lambda
    v = eta1
    kappa_name = kappa
    mob_name = 1
  []
  [mult_ACBulkF_2]
    type = KKSMultiACBulkF
    variable = lambda
    Fj_names = 'F1 F2 '
    hj_names = 'h1 h2 '
    gi_name = g2
    eta_i = eta2
    wi = 0.2
    mob_name = 1
    coupled_variables = 'c1 c2 eta1 '
  []
  [mult_ACBulkC_2]
    type = KKSMultiACBulkC
    variable = lambda
    Fj_names = 'F1 F2'
    hj_names = 'h1 h2'
    cj_names = 'c1 c2'
    eta_i = eta2
    coupled_variables = 'eta1 '
    mob_name = 1
  []
  [mult_CoupledACint_2]
    type = SimpleCoupledACInterface
    variable = lambda
    v = eta2
    kappa_name = kappa
    mob_name = 1
  []

  # Kernels for constraint equation eta1 + eta2 = 1
  # eta2 is the nonlinear variable for the constraint equation
  [eta2reaction]
    type = MatReaction
    variable = eta2
    mob_name = 1
  []
  [eta1reaction]
    type = MatReaction
    variable = eta2
    v = eta1
    mob_name = 1
  []
  [one]
    type = BodyForce
    variable = eta2
    value = -1.0
  []

  # Phase concentration constraints
  [chempot12]
    type = KKSPhaseChemicalPotential
    variable = c1
    cb = c2
    fa_name = F1
    fb_name = F2
  []
  [phaseconcentration]
    type = KKSMultiPhaseConcentration
    variable = c2
    cj = 'c1 c2'
    hj_names = 'h1 h2'
    etas = 'eta1 eta2'
    c = c
  []
[]

[AuxKernels]
  [Fglobal_total]
    type = KKSMultiFreeEnergy
    Fj_names = 'F1 F2 '
    hj_names = 'h1 h2 '
    gj_names = 'g1 g2 '
    variable = Fglobal
    w = 0.2
    interfacial_vars = 'eta1  eta2 '
    kappa_names      = 'kappa kappa'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -sub_pc_factor_shift_type'
  petsc_options_value = 'lu       nonzero'
  l_max_its = 30
  nl_max_its = 10
  l_tol = 1.0e-4
  nl_rel_tol = 1.0e-10
  nl_abs_tol = 1.0e-11

  num_steps = 35
  dt = 10
[]

[VectorPostprocessors]
  [c]
    type = LineValueSampler
    variable = c
    start_point = '0 0 0'
    end_point = '5 0 0'
    num_points = 21
    sort_by = x
  []
[]

[Outputs]
  csv = true
  execute_on = FINAL
[]
