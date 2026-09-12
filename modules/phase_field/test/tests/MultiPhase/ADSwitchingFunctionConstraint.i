
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
  nz = 0
  xmin = -5
  xmax = 5
  ymin = -5
  ymax = 5
  elem_type = QUAD4
[]

[Variables]
  [c]
  []
  [eta1]
  []
  [eta2]
  []
  [eta3]
  []
  [lambda]
  []
  [c1]
    initial_condition = .1
  []
  [c2]
    initial_condition = .8
  []
  [c3]
    initial_condition = .5
  []
[]

[ICs]
  [c_ic]
    type = FunctionIC
    function = c_func
    variable = c
  []
  [eta1_ic]
    type = FunctionIC
    function = eta1_func
    variable = eta1
  []
  [eta2_ic]
    type = FunctionIC
    function = eta2_func
    variable = eta2
  []
  [eta3_ic]
    type = FunctionIC
    function = eta3_func
    variable = eta3
  []
[]

[Functions]
  [eta1_func]
    type = ParsedFunction
    expression = 'if((x-2.5)^2+(y-2.5)^2<1.25^2, 1, 0) +
                  if((x)^2+(y)^2<.675^2, 1, 0)'
  []
  [eta2_func]
    type = ParsedFunction
    expression = 'if((x+2.5)^2+(y+2.5)^2<.875^2, 1, 0) +
                  if((x-1.5)^2+(y+2)^2<.425^2, 1, 0)'
  []
  [eta3_func]
    type = ParsedFunction
    expression = '1 - e1 - e2'
    symbol_names = 'e1 e2'
    symbol_values = 'eta1_func eta2_func'
  []
  [c_func]
    type = ParsedFunction
    expression = '.1*e1 + .8*e2 + .5*e3'
    symbol_names = 'e1 e2 e3'
    symbol_values = 'eta1_func eta2_func eta3_func'
  []
[]

[Kernels]
  [c_dt]
    type = ADTimeDerivative
    variable = c
  []
  [eta1_dt]
    type = ADTimeDerivative
    variable = eta1
  []
  [eta2_dt]
    type = ADTimeDerivative
    variable = eta2
  []
  [eta3_dt]
    type = ADTimeDerivative
    variable = eta3
  []
  [c1_diff]
    type = ADMatDiffusion
    variable = c
    diffusivity = Dh1
    v = c1
  []
  [c2_diff]
    type = ADMatDiffusion
    variable = c
    diffusivity = Dh2
    v = c2
  []
  [c3_diff]
    type = ADMatDiffusion
    variable = c
    diffusivity = Dh3
    v = c3
  []
  # eta1 AC
  [ACBF1]
    type = ADKKSMultiACBulkF
    variable = eta1
    Fj_names = 'f1 f2 f3'
    hj_names = 'h1 h2 h3'
    gi_name = g1
    eta_i = eta1
    wi = 1
  []
  [ACBC1]
    type = ADKKSMultiACBulkC
    Fj_names = 'f1 f2 f3'
    cj_names = 'c1 c2 c3'
    eta_i = eta1
    hj_names = 'h1 h2 h3'
    variable = eta1
  []
  [ACI1]
    type = ADACInterface
    variable = eta1
    kappa_name = kappa
    variable_L = false
  []
  # eta2 AC
  [ACBF2]
    type = ADKKSMultiACBulkF
    variable = eta2
    Fj_names = 'f1 f2 f3'
    hj_names = 'h1 h2 h3'
    gi_name = g2
    eta_i = eta2
    wi = 1
  []
  [ACBC2]
    type = ADKKSMultiACBulkC
    Fj_names = 'f1 f2 f3'
    cj_names = 'c1 c2 c3'
    eta_i = eta2
    hj_names = 'h1 h2 h3'
    variable = eta2
  []
  [ACI2]
    type = ADACInterface
    variable = eta2
    kappa_name = kappa
    variable_L = false
  []
  # eta3 AC
  [ACBF3]
    type = ADKKSMultiACBulkF
    variable = eta3
    Fj_names = 'f1 f2 f3'
    hj_names = 'h1 h2 h3'
    gi_name = g3
    eta_i = eta3
    wi = 1
  []
  [ACBC3]
    type = ADKKSMultiACBulkC
    Fj_names = 'f1 f2 f3'
    cj_names = 'c1 c2 c3'
    eta_i = eta3
    hj_names = 'h1 h2 h3'
    variable = eta3
  []
  [ACI3]
    type = ADACInterface
    variable = eta3
    kappa_name = kappa
    variable_L = false
  []
  # Chem Pot
  [chempot12]
    type = ADKKSPhaseChemicalPotential
    variable = c1
    cb = c2
    fa_name = f1
    fb_name = f2
  []
  [chempot23]
    type = ADKKSPhaseChemicalPotential
    variable = c2
    cb = c3
    fa_name = f2
    fb_name = f3
  []
  [phaseconcentration]
    type = ADKKSMultiPhaseConcentration
    variable = c3
    cj = 'c1 c2 c3'
    hj_names = 'h1 h2 h3'
    etas = 'eta1 eta2 eta3'
    c = c
  []
  # Lagrange Constraints
  [LGeta1]
    type = ADSwitchingFunctionConstraintEta
    lambda = lambda
    variable = eta1
    h_name = h1
  []
  [LGeta2]
    type = ADSwitchingFunctionConstraintEta
    lambda = lambda
    variable = eta2
    h_name = h2
  []
  [LGeta3]
    type = ADSwitchingFunctionConstraintEta
    lambda = lambda
    variable = eta3
    h_name = h3
  []
  [LGLagrange]
    type = ADSwitchingFunctionConstraintLagrange
    etas = 'eta1 eta2 eta3'
    variable = lambda
    h_names = 'h1 h2 h3'
    epsilon = 1e-9
  []
[]

[BCs]
  [Periodic]
    [All]
      auto_direction = 'x y'
    []
  []
[]

[Materials]
  [consts]
    type = ADGenericConstantMaterial
    prop_names = 'L D kappa'
    prop_values = '1 1 1'
  []
  [h1]
    type = ADSwitchingFunctionMaterial
    function_name = h1
    eta = eta1
    h_order = HIGH
    outputs = 'exodus'
    output_properties = 'h1'
  []
  [h2]
    type = ADSwitchingFunctionMaterial
    function_name = h2
    eta = eta2
    h_order = HIGH
    outputs = 'exodus'
    output_properties = 'h2'
  []
  [h3]
    type = ADSwitchingFunctionMaterial
    function_name = h3
    eta = eta3
    h_order = HIGH
    outputs = 'exodus'
    output_properties = 'h3'
  []
  [barrier1]
    type = ADBarrierFunctionMaterial
    eta = eta1
    g_order = HIGH
    function_name = g1
  []
  [barrier2]
    type = ADBarrierFunctionMaterial
    eta = eta2
    g_order = HIGH
    function_name = g2
  []
  [barrier3]
    type = ADBarrierFunctionMaterial
    eta = eta3
    g_order = HIGH
    function_name = g3
  []
  [Dh1]
    type = ADDerivativeParsedMaterial
    property_name = Dh1
    material_property_names = 'h1(eta1) D'
    coupled_variables = 'eta1'
    expression = 'D*h1'
  []
  [Dh2]
    type = ADDerivativeParsedMaterial
    property_name = Dh2
    material_property_names = 'h2(eta2) D'
    coupled_variables = 'eta2'
    expression = 'D*h2'
  []
  [Dh3]
    type = ADDerivativeParsedMaterial
    property_name = Dh3
    material_property_names = 'h3(eta3) D'
    coupled_variables = 'eta3'
    expression = 'D*h3'
  []
  [f1]
    type = ADDerivativeParsedMaterial
    property_name = f1
    coupled_variables = 'c1'
    expression = '(c1-.1)^2'
  []
  [f2]
    type = ADDerivativeParsedMaterial
    property_name = f2
    coupled_variables = 'c2'
    expression = '(c2-.8)^2'
  []
  [f3]
    type = ADDerivativeParsedMaterial
    property_name = f3
    coupled_variables = 'c3'
    expression = '(c3-.5)^2'
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  solve_type = 'NEWTON'
  #petsc_options = '-snes_ksp -snes_ksp_ew'
  #petsc_options = '-ksp_monitor_snes_lg-snes_ksp_ew'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu           nonzero   '

  l_max_its = 15
  l_tol = 1.0e-6

  nl_max_its = 50
  nl_rel_tol = 1.0e-8
  nl_abs_tol = 1.0e-10

  start_time = 0.0
  num_steps = 1
  dt = 0.01
  dtmin = 0.01
[]

[Debug]
  # show_var_residual_norms = true
[]

[Outputs]
  exodus = true
[]
