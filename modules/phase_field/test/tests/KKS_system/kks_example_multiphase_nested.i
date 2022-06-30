#
# This test is for the multiphase KKS nested solve
#

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmin = 0
  xmax = 400
[]

[Variables]
  [./c]
  [../]
  [./b]
  [../]
  [./eta1]
  [../]
  [./eta2]
  [../]
  [./eta3]
  [../]
  [./lambda]
  [../]
  [./mu_c]
  [../]
  [./mu_b]
  [../]
[]

[ICs]
  [./c]
    type = NestedBoundingBoxIC
    variable = c
    int_width = 10
    smaller_coordinate_corners = '-20 0 0 -20 0 0'
    larger_coordinate_corners = '100 0 0 200 0 0'
    inside = '0.5  0.2'
    outside = 0.8
  [../]
  [./b]
    type = NestedBoundingBoxIC
    variable = b
    int_width = 10
    smaller_coordinate_corners = '-20 0 0 -20 0 0'
    larger_coordinate_corners = '100 0 0 200 0 0'
    inside = '0.2  0.5'
    outside = 0.1
  [../]
  [./eta1]
    type = NestedBoundingBoxIC
    variable = eta1
    int_width = 10
    smaller_coordinate_corners = '-20 0 0 -20 0 0'
    larger_coordinate_corners = '100 0 0 200 0 0'
    inside = '1  0'
    outside = 0
  [../]
  [./eta2]
    type = NestedBoundingBoxIC
    variable = eta2
    int_width = 10
    smaller_coordinate_corners = '-20 0 0 -20 0 0'
    larger_coordinate_corners = '100 0 0 200 0 0'
    inside = '0  1'
    outside = 0
  [../]
  [./eta3]
    type = NestedBoundingBoxIC
    variable = eta3
    int_width = 10
    smaller_coordinate_corners = '-20 0 0 -20 0 0'
    larger_coordinate_corners = '100 0 0 200 0 0'
    inside = '0  0'
    outside = 1
  [../]
[]

[GlobalParams]
  derivative_order = 2
  evalerror_behavior = error
  enable_ad_cache = false
  enable_jit = false
  epsilon = 1e-4 # used in lambda kernel
  wi = 10
[]

[Materials]
  [./constants]
    type = GenericConstantMaterial
    prop_names  = 'l   kappa  M   L   ptol'
    prop_values = '10   10    1   0.01   1e-4'
  [../]
  [./F1_mat]
    type = DerivativeParsedMaterial
    f_name = F1_mat
    function = '100*c1 + 500*b1 + 20*(1 - c1 - b1) + 4000*(c1*plog(c1, ptol) + b1*plog(b1, ptol) + (1 - c1 - b1)*plog(1 - c1 - b1, ptol))'
    material_property_names = 'c1(eta1,eta2,eta3,b,c) b1(eta1,eta2,eta3,b,c) ptol'
    additional_derivative_symbols = 'c1 b1'
    compute = false
  [../]
  [./F2_mat]
    type = DerivativeParsedMaterial
    f_name = F2_mat
    function = '2*c2 + 200*b2 + 300*(1 - c2 - b2) + 300*(c2*plog(c2, ptol) + b2*plog(b2, ptol) + (1 - c2 - b2)*plog((1 - c2 - b2), ptol))'
    material_property_names = 'c2(eta1,eta2,eta3,b,c) b2(eta1,eta2,eta3,b,c) ptol'
    additional_derivative_symbols = 'c2 b2'
    compute = false
  [../]
  [./F3_mat]
    type = DerivativeParsedMaterial
    f_name = F3_mat
    function = '1000*c3 + 4*b3 + 500*(1 - c3 - b3) + 5*(c3*plog(c3, ptol) + b3*plog(b3, ptol) + (1 - c3 - b3)*plog((1 - c3 - b3), ptol))'
    material_property_names = 'c3(eta1,eta2,eta3,b,c) b3(eta1,eta2,eta3,b,c) ptol'
    additional_derivative_symbols = 'c3 b3'
    compute = false
  [../]
  [./KKSPhaseConcentrationMultiPhaseMaterial]
    type = KKSPhaseConcentrationMultiPhaseMaterial
    global_cs = 'c b'
    all_etas = 'eta1 eta2 eta3'
    hj_names = 'h1 h2 h3'
    ci_names = 'c1 c2 c3
                b1 b2 b3'
    ci_IC = '0.5 0.2 0.8
             0.2 0.5 0.1'
    Fj_material = 'F1_mat F2_mat F3_mat'
    nested_iterations = iter
    min_iterations = 1
    max_iterations = 1000
    absolute_tolerance = 1e-8
    relative_tolerance = 1e-8
  [../]
  [./KKSPhaseConcentrationMultiPhaseDerivatives]
    type = KKSPhaseConcentrationMultiPhaseDerivatives
    global_cs = 'c b'
    all_etas = 'eta1 eta2 eta3'
    Fj_material = 'F1_mat F2_mat F3_mat'
    hj_names = 'h1 h2 h3'
    ci_names = 'c1 c2 c3
                b1 b2 b3'
    [../]
  [./h1]
    type = SwitchingFunctionMaterial
    function_name = h1
    h_order = HIGH
    eta = eta1
  [../]
  [./h2]
    type = SwitchingFunctionMaterial
    function_name = h2
    h_order = HIGH
    eta = eta2
  [../]
  [./h3]
    type = SwitchingFunctionMaterial
    function_name = h3
    h_order = HIGH
    eta = eta3
  [../]
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
[]

[Kernels]
  [./lambda_lagrange]
    type = SwitchingFunctionConstraintLagrange
    variable = lambda
    etas =    'eta1 eta2 eta3'
    h_names = 'h1   h2   h3'
  [../]
  [./eta1_lagrange]
    type = SwitchingFunctionConstraintEta
    variable = eta1
    h_name = h1
    lambda = lambda
    args = 'eta2 eta3'
  [../]
  [./eta2_lagrange]
    type = SwitchingFunctionConstraintEta
    variable = eta2
    h_name = h2
    lambda = lambda
    args = 'eta1 eta3'
  [../]
  [./eta3_lagrange]
    type = SwitchingFunctionConstraintEta
    variable = eta3
    h_name = h3
    lambda = lambda
    args = 'eta1 eta2'
  [../]

  #Kernels for CH equation
  [./diff_time_c]
    type = CoupledTimeDerivative
    variable = mu_c
    v = c
  [../]
  [./CHBulk_c]
    type = NestKKSSplitCHCRes
    variable = c
    all_etas = 'eta1 eta2 eta3'
    global_cs = 'c b'
    w = mu_c
    c1_names = 'c1 b1'
    F1_name = F1_mat
    args = 'eta1 eta2 eta3 b mu_c'
  [../]
  [./ckernel_c]
    type = SplitCHWRes
    variable = mu_c
    mob_name = M
  [../]

  [./diff_time_b]
    type = CoupledTimeDerivative
    variable = mu_b
    v = b
  [../]
  [./CHBulk_b]
    type = NestKKSSplitCHCRes
    variable = b
    all_etas = 'eta1 eta2 eta3'
    global_cs = 'c b'
    w = mu_b
    c1_names = 'c1 b1'
    F1_name = F1_mat
    args = 'eta1 eta2 eta3 c mu_b'
  [../]
  [./ckernel_b]
    type = SplitCHWRes
    variable = mu_b
    mob_name = M
  [../]

  # Kernels for Allen-Cahn equation for eta1
  [./deta1dt]
    type = TimeDerivative
    variable = eta1
  [../]
  [./ACBulkF1]
    type = NestKKSMultiACBulkF
    variable  = eta1
    global_cs = 'c b'
    eta_i = eta1
    all_etas = 'eta1 eta2 eta3'
    ci_names = 'c1 c2 c3
                b1 b2 b3'
    hj_names = 'h1 h2 h3'
    Fj_names = 'F1_mat F2_mat F3_mat'
    gi_name = g1
    mob_name = L
    args      = 'c b eta2 eta3'
  [../]
  [./ACBulkC_eta1]
    type = NestKKSMultiACBulkC
    variable = eta1
    global_cs = 'c b'
    eta_i = eta1
    all_etas = 'eta1 eta2 eta3'
    ci_names = 'c1 c2 c3
                b1 b2 b3'
    hj_names = 'h1 h2 h3'
    Fj_names = 'F1_mat F2_mat F3_mat'
    args      = 'c b eta2 eta3'
  [../]
  [./ACInterface1]
    type = ACInterface
    variable = eta1
    kappa_name = kappa
    args = 'eta2 eta3'
  [../]

  # Kernels for Allen-Cahn equation for eta2
  [./deta2dt]
    type = TimeDerivative
    variable = eta2
  [../]
  [./ACBulkF2]
    type = NestKKSMultiACBulkF
    variable  = eta2
    global_cs = 'c b'
    eta_i = eta2
    all_etas = 'eta1 eta2 eta3'
    ci_names = 'c1 c2 c3
                b1 b2 b3'
    hj_names = 'h1 h2 h3'
    Fj_names = 'F1_mat F2_mat F3_mat'
    gi_name = g2
    mob_name = L
    args      = 'c b eta1 eta3'
  [../]
  [./ACBulkC_eta2]
    type = NestKKSMultiACBulkC
    variable = eta2
    global_cs = 'c b'
    eta_i = eta2
    all_etas = 'eta1 eta2 eta3'
    ci_names = 'c1 c2 c3
                b1 b2 b3'
    hj_names = 'h1 h2 h3'
    Fj_names = 'F1_mat F2_mat F3_mat'
    args      = 'c b eta1 eta3'
  [../]
  [./ACInterface2]
    type = ACInterface
    variable = eta2
    kappa_name = kappa
    args = 'eta1 eta3'
  [../]

  # Kernels for Allen-Cahn equation for eta3
  [./deta3dt]
    type = TimeDerivative
    variable = eta3
  [../]
  [./ACBulkF3]
    type = NestKKSMultiACBulkF
    variable  = eta3
    global_cs = 'c b'
    eta_i = eta3
    all_etas = 'eta1 eta2 eta3'
    ci_names = 'c1 c2 c3
                b1 b2 b3'
    hj_names = 'h1 h2 h3'
    Fj_names = 'F1_mat F2_mat F3_mat'
    gi_name = g3
    mob_name = L
    args      = 'c b eta1 eta2'
  [../]
  [./ACBulkC_eta3]
    type = NestKKSMultiACBulkC
    variable = eta3
    global_cs = 'c b'
    eta_i = eta3
    all_etas = 'eta1 eta2 eta3'
    ci_names = 'c1 c2 c3
                b1 b2 b3'
    hj_names = 'h1 h2 h3'
    Fj_names = 'F1_mat F2_mat F3_mat'
    args      = 'c b eta1 eta2'
  [../]
  [./ACInterface3]
    type = ACInterface
    variable = eta3
    kappa_name = kappa
    args = 'eta1 eta2'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu       superlu_dist'
  nl_abs_tol = 1e-9
  l_tol = 1e-8
  l_max_its = 50
  nl_max_its = 500
  dt = 5e-4
  num_steps = 2
[]

[Preconditioning]
  [./full]
    type = SMP
    full = true
  [../]
[]

[Outputs]
  file_base = kks_example_multiphase_nested
  exodus = true
[]
