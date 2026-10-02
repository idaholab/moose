# Three-phase ternary KKS action test using global phase concentration variables.
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 8
[]

[Modules]
  [PhaseField]
    [KKS]
      phase_concentration_solve = GLOBAL
      phase_names = 'alpha beta gamma'
      order_parameters = 'eta_alpha eta_beta eta_gamma'
      global_concentrations = 'c b'
      free_energies = 'F_alpha F_beta F_gamma'
      switching_functions = 'h_alpha h_beta h_gamma'
      barrier_functions = 'g_alpha g_beta g_gamma'
      barrier_heights = '0 0 0'
      concentration_mobilities = 'M_c M_b'
      order_parameter_mobilities = 'L_alpha L_beta L_gamma'
      kappas = 'kappa_alpha kappa_beta kappa_gamma'
    []
  []
[]

[ICs]
  [c]
    type = FunctionIC
    variable = c
    function = '0.22 + 0.005*x'
  []
  [b]
    type = FunctionIC
    variable = b
    function = '0.25 - 0.004*x'
  []
  [eta_alpha]
    type = FunctionIC
    variable = eta_alpha
    function = '0.70 + 0.005*x'
  []
  [eta_beta]
    type = FunctionIC
    variable = eta_beta
    function = '0.30 - 0.004*x'
  []
  [eta_gamma]
    type = FunctionIC
    variable = eta_gamma
    function = '0.002 + 0.001*x'
  []
  [c_alpha]
    type = FunctionIC
    variable = c_alpha
    function = '0.10 + 0.010*x'
  []
  [c_beta]
    type = FunctionIC
    variable = c_beta
    function = '0.50 - 0.008*x'
  []
  [c_gamma]
    type = FunctionIC
    variable = c_gamma
    function = '0.80 + 0.006*x'
  []
  [b_alpha]
    type = FunctionIC
    variable = b_alpha
    function = '0.20 - 0.007*x'
  []
  [b_beta]
    type = FunctionIC
    variable = b_beta
    function = '0.30 + 0.009*x'
  []
  [b_gamma]
    type = FunctionIC
    variable = b_gamma
    function = '0.60 - 0.005*x'
  []
[]

[Materials]
  [F_alpha]
    type = DerivativeParsedMaterial
    property_name = F_alpha
    coupled_variables = 'c_alpha b_alpha'
    expression = '10*(c_alpha-0.1)^2 + 8*(b_alpha-0.2)^2 + 2*(c_alpha-0.1)*(b_alpha-0.2)'
  []
  [F_beta]
    type = DerivativeParsedMaterial
    property_name = F_beta
    coupled_variables = 'c_beta b_beta'
    expression = '12*(c_beta-0.5)^2 + 9*(b_beta-0.3)^2 + (c_beta-0.5)*(b_beta-0.3)'
  []
  [F_gamma]
    type = DerivativeParsedMaterial
    property_name = F_gamma
    coupled_variables = 'c_gamma b_gamma'
    expression = '11*(c_gamma-0.8)^2 + 7*(b_gamma-0.6)^2 + 1.5*(c_gamma-0.8)*(b_gamma-0.6)'
  []

  [h_alpha]
    type = SwitchingFunctionMaterial
    eta = eta_alpha
    function_name = h_alpha
  []
  [h_beta]
    type = SwitchingFunctionMaterial
    eta = eta_beta
    function_name = h_beta
  []
  [h_gamma]
    type = SwitchingFunctionMaterial
    eta = eta_gamma
    function_name = h_gamma
  []

  [g_alpha]
    type = BarrierFunctionMaterial
    eta = eta_alpha
    function_name = g_alpha
    g_order = SIMPLE
  []
  [g_beta]
    type = BarrierFunctionMaterial
    eta = eta_beta
    function_name = g_beta
    g_order = SIMPLE
  []
  [g_gamma]
    type = BarrierFunctionMaterial
    eta = eta_gamma
    function_name = g_gamma
    g_order = SIMPLE
  []

  [constants]
    type = GenericConstantMaterial
    prop_names = 'M_c M_b L_alpha L_beta L_gamma kappa_alpha kappa_beta kappa_gamma'
    prop_values = '1   1   1       1      1       1           1          1'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  num_steps = 1
  dt = 0.01
  # Resolve the coupled constraint residual well below the initial perturbation.
  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-10
  nl_max_its = 20
  # Use a direct solve for this small saddle-point regression system.
  petsc_options_iname = '-ksp_type -pc_type -pc_factor_shift_type'
  petsc_options_value = 'preonly   lu       NONZERO'
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Postprocessors]
  [c_l2]
    type = ElementL2Norm
    variable = c
  []
  [b_l2]
    type = ElementL2Norm
    variable = b
  []
  [eta_alpha_l2]
    type = ElementL2Norm
    variable = eta_alpha
  []
  [eta_beta_l2]
    type = ElementL2Norm
    variable = eta_beta
  []
  [eta_gamma_l2]
    type = ElementL2Norm
    variable = eta_gamma
  []
[]

[Outputs]
  csv = true
  file_base = kks_3phase_ternary
[]
