# Three-phase ternary KKS example using nested phase concentrations.
[Mesh]
  type = GeneratedMesh
  dim = 2
  # Use one unit square element per display pixel on the 50 by 50 domain.
  nx = 50
  ny = 50
  xmax = 50
  ymax = 50
[]

[BCs]
  [Periodic]
    [all]
      auto_direction = 'x y'
    []
  []
[]

[Modules]
  [PhaseField]
    [KKS]
      phase_concentration_solve = NESTED
      phase_names = 'alpha beta gamma'
      order_parameters = 'eta_alpha eta_beta eta_gamma'
      global_concentrations = 'c b'
      free_energies = 'F_alpha F_beta F_gamma'
      switching_functions = 'h_alpha h_beta h_gamma'
      barrier_functions = 'g_alpha g_beta g_gamma'
      # Unit barriers and gradient coefficients give diffuse, mobile interfaces.
      barrier_heights = '1 1 1'
      concentration_mobilities = 'M_c M_b'
      order_parameter_mobilities = 'L_alpha L_beta L_gamma'
      kappas = 'kappa_alpha kappa_beta kappa_gamma'
      phase_concentration_initial_values = 'c_alpha c_beta c_gamma b_alpha b_beta b_gamma'
    []
  []
[]

[AuxVariables]
  [c_alpha]
    family = LAGRANGE
    order = FIRST
  []
  [c_beta]
    family = LAGRANGE
    order = FIRST
  []
  [c_gamma]
    family = LAGRANGE
    order = FIRST
  []
  [b_alpha]
    family = LAGRANGE
    order = FIRST
  []
  [b_beta]
    family = LAGRANGE
    order = FIRST
  []
  [b_gamma]
    family = LAGRANGE
    order = FIRST
  []
[]

[ICs]
  [c]
    type = FunctionIC
    variable = c
    # Each phase starts 10 percent above its c free-energy minimum.
    function = 'ra:=sqrt((x-14)^2+(y-25)^2); a:=if(ra<=6.5,1,if(ra<9.5,0.5*(1+cos(pi*(ra-6.5)/3)),0)); rb:=sqrt((x-36)^2+(y-25)^2); bt:=if(rb<=8.5,1,if(rb<11.5,0.5*(1+cos(pi*(rb-8.5)/3)),0)); 0.88-0.77*a-0.33*bt'
  []
  [b]
    type = FunctionIC
    variable = b
    # Each phase starts 10 percent above its b free-energy minimum.
    function = 'ra:=sqrt((x-14)^2+(y-25)^2); a:=if(ra<=6.5,1,if(ra<9.5,0.5*(1+cos(pi*(ra-6.5)/3)),0)); rb:=sqrt((x-36)^2+(y-25)^2); bt:=if(rb<=8.5,1,if(rb<11.5,0.5*(1+cos(pi*(rb-8.5)/3)),0)); 0.66-0.44*a-0.33*bt'
  []
  [eta_alpha]
    type = SmoothCircleIC
    variable = eta_alpha
    # Unequal radii provide a curvature-driven reason for both boundaries to move.
    x1 = 14
    y1 = 25
    radius = 8
    invalue = 1
    outvalue = 0
    # Three mesh cells resolve each diffuse interface.
    int_width = 3
    profile = COS
  []
  [eta_beta]
    type = SmoothCircleIC
    variable = eta_beta
    x1 = 36
    y1 = 25
    radius = 10
    invalue = 1
    outvalue = 0
    int_width = 3
    profile = COS
  []
  [eta_gamma]
    type = SpecifiedSmoothCircleIC
    variable = eta_gamma
    # The third phase is the matrix complementary to both circles.
    x_positions = '14 36'
    y_positions = '25 25'
    z_positions = '0 0'
    radii = '8 10'
    invalue = 0
    outvalue = 1
    int_width = 3
    profile = COS
  []
  [c_alpha]
    type = ConstantIC
    variable = c_alpha
    value = 0.11
  []
  [c_beta]
    type = ConstantIC
    variable = c_beta
    value = 0.55
  []
  [c_gamma]
    type = ConstantIC
    variable = c_gamma
    value = 0.88
  []
  [b_alpha]
    type = ConstantIC
    variable = b_alpha
    value = 0.22
  []
  [b_beta]
    type = ConstantIC
    variable = b_beta
    value = 0.33
  []
  [b_gamma]
    type = ConstantIC
    variable = b_gamma
    value = 0.66
  []
[]

[Materials]
  [F_alpha]
    type = DerivativeParsedMaterial
    property_name = F_alpha
    material_property_names = 'c_alpha b_alpha'
    additional_derivative_symbols = 'c_alpha b_alpha'
    expression = '10*(c_alpha-0.1)^2 + 8*(b_alpha-0.2)^2 + 2*(c_alpha-0.1)*(b_alpha-0.2)'
    compute = false
  []
  [F_beta]
    type = DerivativeParsedMaterial
    property_name = F_beta
    material_property_names = 'c_beta b_beta'
    additional_derivative_symbols = 'c_beta b_beta'
    expression = '12*(c_beta-0.5)^2 + 9*(b_beta-0.3)^2 + (c_beta-0.5)*(b_beta-0.3)'
    compute = false
  []
  [F_gamma]
    type = DerivativeParsedMaterial
    property_name = F_gamma
    material_property_names = 'c_gamma b_gamma'
    additional_derivative_symbols = 'c_gamma b_gamma'
    expression = '11*(c_gamma-0.8)^2 + 7*(b_gamma-0.6)^2 + 1.5*(c_gamma-0.8)*(b_gamma-0.6)'
    compute = false
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
  # Twenty implicit increments expose interface motion without making the example unwieldy.
  num_steps = 20
  dt = 0.01
  # Resolve the coupled constraint residual well below the 10 percent disequilibrium.
  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-10
  nl_max_its = 20
  # Use a direct solve for the saddle-point system generated by the Lagrange constraint.
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
  file_base = kks_example_3phase_ternary_nested
  [csv]
    type = CSV
  []
  [exodus]
    type = Exodus
  []
[]
