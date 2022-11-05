#
# Fig. 6 input for 10.1016/j.commatsci.2017.02.017
# D. Schwen et al./Computational Materials Science 132 (2017) 36-45
# Three phase interface simulation demonstrating the interfacial stability
# w.r.t. formation of a tspurious third phase
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 120
  ny = 120
  nz = 0
  xmin = 0
  xmax = 40
  ymin = 0
  ymax = 40
  zmin = 0
  zmax = 0
  elem_type = QUAD4
[]

[Variables]
  # concentration
  [./c]
  [../]

  # order parameter 1
  [./eta1]
  [../]

  # order parameter 2
  [./eta2]
  [../]

  # order parameter 3
  [./eta3]
  [../]

  # phase concentration 1
  [./c1]
    initial_condition = 0.4
  [../]

  # phase concentration 2
  [./c2]
    initial_condition = 0.5
  [../]

  # phase concentration 3
  [./c3]
    initial_condition = 0.8
  [../]

  # Lagrange multiplier
  [./lambda]
    initial_condition = 0.0
  [../]
[]

[AuxVariables]
  [./T]
    [./InitialCondition]
      type = FunctionIC
      function = 'x-10'
    [../]
  [../]
[]


[Functions]
  [./ic_func_eta1]
    type = ParsedFunction
    expression = '0.5*(1.0+tanh((x-10)/sqrt(2.0))) * 0.5*(1.0+tanh((y-10)/sqrt(2.0)))'
  [../]
  [./ic_func_eta2]
    type = ParsedFunction
    expression = '0.5*(1.0-tanh((x-10)/sqrt(2.0)))'
  [../]
  [./ic_func_eta3]
    type = ParsedFunction
    expression = '1 - 0.5*(1.0-tanh((x-10)/sqrt(2.0)))
              - 0.5*(1.0+tanh((x-10)/sqrt(2.0))) * 0.5*(1.0+tanh((y-10)/sqrt(2.0)))'
  [../]
  [./ic_func_c]
    type = ParsedFunction
    expression = '0.5 * 0.5*(1.0-tanh((x-10)/sqrt(2.0)))
              + 0.4 * 0.5*(1.0+tanh((x-10)/sqrt(2.0))) * 0.5*(1.0+tanh((y-10)/sqrt(2.0)))
              + 0.8 * (1 - 0.5*(1.0-tanh((x-10)/sqrt(2.0)))
                        - 0.5*(1.0+tanh((x-10)/sqrt(2.0))) * 0.5*(1.0+tanh((y-10)/sqrt(2.0))))'
  [../]
[]

[ICs]
  [./eta1]
    variable = eta1
    type = FunctionIC
    function = ic_func_eta1
  [../]
  [./eta2]
    variable = eta2
    type = FunctionIC
    function = ic_func_eta2
  [../]
  [./eta3]
    variable = eta3
    type = FunctionIC
    function = ic_func_eta3
  [../]
  [./c]
    variable = c
    type = FunctionIC
    function = ic_func_c
  [../]
[]

[Materials]
  # simple toy free energies
  [./f1]
    type = DerivativeParsedMaterial
    property_name = F1
    coupled_variables = 'c1'
    expression = '20*(c1-0.4)^2'
  [../]
  [./f2]
    type = DerivativeParsedMaterial
    property_name = F2
    coupled_variables = 'c2 T'
    expression = '20*(c2-0.5)^2 + 0.01*T'
  [../]
  [./f3]
    type = DerivativeParsedMaterial
    property_name = F3
    coupled_variables = 'c3'
    expression = '20*(c3-0.8)^2'
  [../]

  # Switching functions for each phase
  # h1(eta1, eta2, eta3)
  [./h1]
    type = SwitchingFunction3PhaseMaterial
    eta_i = eta1
    eta_j = eta2
    eta_k = eta3
    f_name = h1
  [../]
  # h2(eta1, eta2, eta3)
  [./h2]
    type = SwitchingFunction3PhaseMaterial
    eta_i = eta2
    eta_j = eta3
    eta_k = eta1
    f_name = h2
  [../]
  # h3(eta1, eta2, eta3)
  [./h3]
    type = SwitchingFunction3PhaseMaterial
    eta_i = eta3
    eta_j = eta1
    eta_k = eta2
    f_name = h3
  [../]

  # Coefficients for diffusion equation
  [./Dh1]
    type = DerivativeParsedMaterial
    material_property_names = 'D h1'
    expression = D*h1
    property_name = Dh1
  [../]
  [./Dh2]
    type = DerivativeParsedMaterial
    material_property_names = 'D h2'
    expression = D*h2
    property_name = Dh2
  [../]
  [./Dh3]
    type = DerivativeParsedMaterial
    material_property_names = 'D h3'
    expression = D*h3
    property_name = Dh3
  [../]

  # Barrier functions for each phase
  [./g1]
    type = BarrierFunctionMaterial
    g_order = SIMPLE
    eta = eta1
    function_name = g1
  [../]
  [./g2]
    type = BarrierFunctionMaterial
    g_order = SIMPLE
    eta = eta2
    function_name = g2
  [../]
  [./g3]
    type = BarrierFunctionMaterial
    g_order = SIMPLE
    eta = eta3
    function_name = g3
  [../]

  # constant properties
  [./constants]
    type = GenericConstantMaterial
    prop_names  = 'L   kappa  D'
    prop_values = '1.0 1.0    1'
  [../]
[]

[Kernels]
  #Kernels for diffusion equation
  [./diff_time]
    type = TimeDerivative
    variable = c
  [../]
  [./diff_c1]
    type = MatDiffusion
    variable = c
    diffusivity = Dh1
    v = c1
  [../]
  [./diff_c2]
    type = MatDiffusion
    variable = c
    diffusivity = Dh2
    v = c2
  [../]
  [./diff_c3]
    type = MatDiffusion
    variable = c
    diffusivity = Dh3
    v = c3
  [../]

  # Kernels for Allen-Cahn equation for eta1
  [./deta1dt]
    type = TimeDerivative
    variable = eta1
  [../]
  [./ACBulkF1]
    type = KKSMultiACBulkF
    variable  = eta1
    Fj_names  = 'F1 F2 F3'
    hj_names  = 'h1 h2 h3'
    gi_name   = g1
    eta_i     = eta1
    wi        = 1.0
    args      = 'c1 c2 c3 eta2 eta3'
  [../]
  [./ACBulkC1]
    type = KKSMultiACBulkC
    variable  = eta1
    Fj_names  = 'F1 F2 F3'
    hj_names  = 'h1 h2 h3'
    cj_names  = 'c1 c2 c3'
    eta_i     = eta1
    args      = 'eta2 eta3'
  [../]
  [./ACInterface1]
    type = ACInterface
    variable = eta1
    kappa_name = kappa
  [../]
  [./multipler1]
    type = MatReaction
    variable = eta1
    v = lambda
    mob_name = L
  [../]

  # Kernels for Allen-Cahn equation for eta2
  [./deta2dt]
    type = TimeDerivative
    variable = eta2
  [../]
  [./ACBulkF2]
    type = KKSMultiACBulkF
    variable  = eta2
    Fj_names  = 'F1 F2 F3'
    hj_names  = 'h1 h2 h3'
    gi_name   = g2
    eta_i     = eta2
    wi        = 1.0
    args      = 'c1 c2 c3 eta1 eta3'
  [../]
  [./ACBulkC2]
    type = KKSMultiACBulkC
    variable  = eta2
    Fj_names  = 'F1 F2 F3'
    hj_names  = 'h1 h2 h3'
    cj_names  = 'c1 c2 c3'
    eta_i     = eta2
    args      = 'eta1 eta3'
  [../]
  [./ACInterface2]
    type = ACInterface
    variable = eta2
    kappa_name = kappa
  [../]
  [./multipler2]
    type = MatReaction
    variable = eta2
    v = lambda
    mob_name = L
  [../]

  # Kernels for the Lagrange multiplier equation
  [./mult_lambda]
    type = MatReaction
    variable = lambda
    mob_name = 3
  [../]
  [./mult_ACBulkF_1]
    type = KKSMultiACBulkF
    variable  = lambda
    Fj_names  = 'F1 F2 F3'
    hj_names  = 'h1 h2 h3'
    gi_name   = g1
    eta_i     = eta1
    wi        = 1.0
    mob_name  = 1
    args      = 'c1 c2 c3 eta2 eta3'
  [../]
  [./mult_ACBulkC_1]
    type = KKSMultiACBulkC
    variable  = lambda
    Fj_names  = 'F1 F2 F3'
    hj_names  = 'h1 h2 h3'
    cj_names  = 'c1 c2 c3'
    eta_i     = eta1
    args      = 'eta2 eta3'
    mob_name  = 1
  [../]
  [./mult_CoupledACint_1]
    type = SimpleCoupledACInterface
    variable = lambda
    v = eta1
    kappa_name = kappa
    mob_name = 1
  [../]
  [./mult_ACBulkF_2]
    type = KKSMultiACBulkF
    variable  = lambda
    Fj_names  = 'F1 F2 F3'
    hj_names  = 'h1 h2 h3'
    gi_name   = g2
    eta_i     = eta2
    wi        = 1.0
    mob_name  = 1
    args      = 'c1 c2 c3 eta1 eta3'
  [../]
  [./mult_ACBulkC_2]
    type = KKSMultiACBulkC
    variable  = lambda
    Fj_names  = 'F1 F2 F3'
    hj_names  = 'h1 h2 h3'
    cj_names  = 'c1 c2 c3'
    eta_i     = eta2
    args      = 'eta1 eta3'
    mob_name  = 1
  [../]
  [./mult_CoupledACint_2]
    type = SimpleCoupledACInterface
    variable = lambda
    v = eta2
    kappa_name = kappa
    mob_name = 1
  [../]
  [./mult_ACBulkF_3]
    type = KKSMultiACBulkF
    variable  = lambda
    Fj_names  = 'F1 F2 F3'
    hj_names  = 'h1 h2 h3'
    gi_name   = g3
    eta_i     = eta3
    wi        = 1.0
    mob_name  = 1
    args      = 'c1 c2 c3 eta1 eta2'
  [../]
  [./mult_ACBulkC_3]
    type = KKSMultiACBulkC
    variable  = lambda
    Fj_names  = 'F1 F2 F3'
    hj_names  = 'h1 h2 h3'
    cj_names  = 'c1 c2 c3'
    eta_i     = eta3
    args      = 'eta1 eta2'
    mob_name  = 1
  [../]
  [./mult_CoupledACint_3]
    type = SimpleCoupledACInterface
    variable = lambda
    v = eta3
    kappa_name = kappa
    mob_name = 1
  [../]

  # Kernels for constraint equation eta1 + eta2 + eta3 = 1
  # eta3 is the nonlinear variable for the constraint equation
  [./eta3reaction]
    type = MatReaction
    variable = eta3
    mob_name = 1
  [../]
  [./eta1reaction]
    type = MatReaction
    variable = eta3
    v = eta1
    mob_name = 1
  [../]
  [./eta2reaction]
    type = MatReaction
    variable = eta3
    v = eta2
    mob_name = 1
  [../]
  [./one]
    type = BodyForce
    variable = eta3
    value = -1.0
  [../]

  # Phase concentration constraints
  [./chempot12]
    type = KKSPhaseChemicalPotential
    variable = c1
    cb       = c2
    fa_name  = F1
    fb_name  = F2
  [../]
  [./chempot23]
    type = KKSPhaseChemicalPotential
    variable = c2
    cb       = c3
    fa_name  = F2
    fb_name  = F3
  [../]
  [./phaseconcentration]
    type = KKSMultiPhaseConcentration
    variable = c3
    cj = 'c1 c2 c3'
    hj_names = 'h1 h2 h3'
    etas = 'eta1 eta2 eta3'
    c = c
  [../]
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

  num_steps = 1000
  [./TimeStepper]
    type = IterationAdaptiveDT
    dt = 0.2
    optimal_iterations = 10
    iteration_window = 2
  [../]
[]

[Preconditioning]
  active = 'full'
  [./full]
    type = SMP
    full = true
  [../]
  [./mydebug]
    type = FDP
    full = true
  [../]
[]

[Outputs]
  exodus = true
  checkpoint = true
  print_linear_residuals = false
  [./csv]
    type = CSV
    execute_on = 'final'
  [../]
[]

#[VectorPostprocessors]
#  [./c]
#    type =  LineValueSampler
#    start_point = '-25 0 0'
#    end_point = '25 0 0'
#    variable = c
#    num_points = 151
#    sort_by =  id
#    execute_on = timestep_end
#  [../]
#  [./eta1]
#    type =  LineValueSampler
#    start_point = '-25 0 0'
#    end_point = '25 0 0'
#    variable = eta1
#    num_points = 151
#    sort_by =  id
#    execute_on = timestep_end
#  [../]
#  [./eta2]
#    type =  LineValueSampler
#    start_point = '-25 0 0'
#    end_point = '25 0 0'
#    variable = eta2
#    num_points = 151
#    sort_by =  id
#    execute_on = timestep_end
#  [../]
#  [./eta3]
#    type =  LineValueSampler
#    start_point = '-25 0 0'
#    end_point = '25 0 0'
#    variable = eta3
#    num_points = 151
#    sort_by =  id
#    execute_on = timestep_end
#  [../]
#[]
