#
# This test is for the 2-phase KKS nested solve
#

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmin = 0
  xmax = 100
[]

[Variables]
  [./c]
  [../]
  [./b]
  [../]
  [./eta]
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
    int_width = 2
    smaller_coordinate_corners = '-4 0 0'
    larger_coordinate_corners = '30 0 0'
    inside = 0.4
    outside = 0.6
  [../]
  [./b]
    type = NestedBoundingBoxIC
    variable = b
    int_width = 2
    smaller_coordinate_corners = '-4 0 0'
    larger_coordinate_corners = '30 0 0'
    inside = 0.1
    outside = 0.2
  [../]
  [./eta]
    type = NestedBoundingBoxIC
    variable = eta
    int_width = 2
    smaller_coordinate_corners = '-4 0 0'
    larger_coordinate_corners = '30 0 0'
    inside = '0'
    outside = 1
  [../]
[]

[GlobalParams]
  derivative_order = 2
  evalerror_behavior = error
  enable_ad_cache = false
  enable_jit = false
[]

[Materials]
  [./const]
    type = GenericConstantMaterial
    prop_names = 'kappa  L  l  M  ptol'
    prop_values = '0.1   2  2  1  1e-4'
  [../]
  [./F1_mat]
    type = DerivativeParsedMaterial
    f_name = F1_mat
    function = '10*c1 + 500*b1 + 40*(1 - c1 - b1) + 400*(c1*plog(c1, ptol) + b1*plog(b1, ptol) + (1 - c1 - b1)*plog((1 - c1 - b1), ptol))'
    material_property_names = 'c1 b1 ptol'
    additional_derivative_symbols = 'c1 b1'
    compute = false
  [../]
  [./F2_mat]
    type = DerivativeParsedMaterial
    f_name = F2_mat
    function = '100*c2 + 4*b2 + 300*(1 - c2 - b2) + 500*(c2*plog(c2, ptol) + b2*plog(b2, ptol) + (1 - c2 - b2)*plog((1 - c2 - b2), ptol))'
    material_property_names = 'c2 b2 ptol'
    additional_derivative_symbols = 'c2 b2'
    compute = false
  [../]
  [./PhaseConcentrationMaterial]
    type = KKSPhaseConcentrationMaterial
    global_cs = 'c b'
    ci_names = 'c1 c2 b1 b2'
    ci_IC = '0.4 0.6 0.1 0.2'
    Fa_material = F1_mat
    Fb_material = F2_mat
    h_name = h
    min_iterations = 1
    max_iterations = 100
    absolute_tolerance = 1e-9
    relative_tolerance = 1e-9
  [../]
  [./PhaseConcentrationDerivatives]
    type = KKSPhaseConcentrationDerivatives
    global_cs = 'c b'
    eta = eta
    ci_names = 'c1 c2
                b1 b2'
    Fa_material = F1_mat
    Fb_material = F2_mat
    h_name = h
  [../]
  [./h]
    type = SwitchingFunctionMaterial
    function_name = h
    h_order = HIGH
    eta = eta
  [../]
  [./g]
    type = BarrierFunctionMaterial
    g_order = SIMPLE
    eta = eta
  [../]
[]

[Kernels]
  # Concentration c
  [./CHBulk_c]
    type = NestKKSSplitCHCRes
    variable = c
    global_cs = 'c b'
    w = mu_c
    all_etas = eta
    c1_names = 'c1 b1'
    F1_name = F1_mat
    args = 'eta b mu_c'
  [../]
  [./dcdt_c]
    type = CoupledTimeDerivative
    variable = mu_c
    v = c
  [../]
  [./ckernel_c]
    type = SplitCHWRes
    variable = mu_c
    mob_name = M
    args = eta
  [../]

  # Concentration b
  [./CHBulk_b]
    type = NestKKSSplitCHCRes
    variable = b
    global_cs = 'c b'
    w = mu_b
    all_etas = eta
    c1_names = 'c1 b1'
    F1_name = F1_mat
    args = 'eta c mu_b'
  [../]
  [./dcdt_b]
    type = CoupledTimeDerivative
    variable = mu_b
    v = b
  [../]
  [./ckernel_b]
    type = SplitCHWRes
    variable = mu_b
    mob_name = M
    args = eta
  [../]

  # Phase parameter
  [./ACBulkF]
    type = NestKKSACBulkF
    variable = eta
    global_cs = 'c b'
    ci_names = 'c1 c2
                b1 b2'
    fa_name = F1_mat
    fb_name = F2_mat
    g_name = g
    h_name = h
    mob_name = L
    w = 1
    args = 'c b'
  [../]
  [./ACBulkC]
    type = NestKKSACBulkC
    variable = eta
    global_cs = 'c b'
    ci_names = 'c1 c2
                b1 b2'
    fa_name = F1_mat
    h_name = h
    mob_name = L
    args = 'c b'
  [../]
  [./ACInterface]
    type = ACInterface
    variable = eta
    kappa_name = kappa
    mob_name = L
  [../]
  [./detadt]
    type = TimeDerivative
    variable = eta
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  nl_abs_tol = 1e-9
  nl_rel_tol = 1e-8
  l_tol = 1e-4
  l_max_its = 50
  nl_max_its = 15
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu       superlu_dist'
  num_steps = 2
  dt = 1e-3
[]

[Preconditioning]
  [./full]
    type = SMP
    full = true
  [../]
[]

[Outputs]
  file_base = kks_example_nested
  exodus = true
[]
