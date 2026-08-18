# Axisymmetric (RZ) mortar normal contact used to verify the node-based Lagrange
# multiplier scaling of Popp et al. (2013) in RZ coordinates. An inner cylinder
# (secondary, r in [0,1]) is compressed by a coaxial outer annulus (primary,
# r in [1,2]). The annulus is axially taller than the inner cylinder, so the
# entire secondary surface projects onto the primary: coverage is full and the
# scaling factor kappa_j must equal 1 at every secondary node. With kappa_j = 1
# the scaled and unscaled formulations are identical -- including the stored
# (scaled) Lagrange multiplier zhat_j = kappa_j lambda_j -- so this input runs
# with 'use_nodal_scaling' both on and off against the same gold. That match is
# the check that the RZ per-node integral int_e N_j (which differs from |e|/n in
# RZ) is used as the kappa_j denominator.

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [inner]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0.0
    xmax = 1.0
    ymin = 0.0
    ymax = 1.0
    nx = 3
    ny = 5
    elem_type = QUAD4
  []
  [inner_sidesets]
    type = RenameBoundaryGenerator
    input = inner
    old_boundary = '0 1 2 3'
    new_boundary = '10 11 12 13'
  []
  [inner_id]
    type = SubdomainIDGenerator
    input = inner_sidesets
    subdomain_id = 1
  []

  [outer]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 1.0
    xmax = 2.0
    ymin = -0.2
    ymax = 1.2
    nx = 3
    ny = 8
    elem_type = QUAD4
  []
  [outer_sidesets]
    type = RenameBoundaryGenerator
    input = outer
    old_boundary = '0 1 2 3'
    new_boundary = '20 21 22 23'
  []
  [outer_id]
    type = SubdomainIDGenerator
    input = outer_sidesets
    subdomain_id = 2
  []

  [combined_mesh]
    type = MeshCollectionGenerator
    inputs = 'inner_id outer_id'
  []

  [secondary_lower]
    type = LowerDBlockFromSidesetGenerator
    input = combined_mesh
    sidesets = '11'
    new_block_id = '10001'
    new_block_name = 'secondary_lower'
  []
  [primary_lower]
    type = LowerDBlockFromSidesetGenerator
    input = secondary_lower
    sidesets = '23'
    new_block_id = '10000'
    new_block_name = 'primary_lower'
  []

  coord_type = RZ
[]

[Variables]
  [normal_lm]
    block = 'secondary_lower'
    use_dual = true
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    strain = SMALL
    add_variables = true
    block = '1 2'
  []
[]

[Functions]
  [compress]
    type = PiecewiseLinear
    x = '0 0.5'
    y = '0 -0.02'
  []
[]

[BCs]
  [axis]
    type = DirichletBC
    variable = disp_x
    boundary = 13
    value = 0.0
  []
  [inner_bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = 10
    value = 0.0
  []
  [outer_push_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 21
    function = compress
  []
  [outer_fix_y]
    type = DirichletBC
    variable = disp_y
    boundary = 21
    value = 0.0
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    block = '1 2'
    youngs_modulus = 1.0e6
    poissons_ratio = 0.3
  []
  [stress]
    type = ComputeLinearElasticStress
    block = '1 2'
  []
[]

[UserObjects]
  [weighted_gap_uo]
    type = LMWeightedGapUserObject
    primary_boundary = '23'
    secondary_boundary = '11'
    primary_subdomain = 'primary_lower'
    secondary_subdomain = 'secondary_lower'
    lm_variable = normal_lm
    correct_edge_dropping = true
    disp_x = disp_x
    disp_y = disp_y
  []
[]

[Constraints]
  [normal_lm]
    type = ComputeWeightedGapLMMechanicalContact
    primary_boundary = '23'
    secondary_boundary = '11'
    primary_subdomain = 'primary_lower'
    secondary_subdomain = 'secondary_lower'
    variable = normal_lm
    disp_x = disp_x
    disp_y = disp_y
    use_displaced_mesh = true
    correct_edge_dropping = true
    weighted_gap_uo = weighted_gap_uo
  []
  [normal_x]
    type = NormalMortarMechanicalContact
    primary_boundary = '23'
    secondary_boundary = '11'
    primary_subdomain = 'primary_lower'
    secondary_subdomain = 'secondary_lower'
    variable = normal_lm
    secondary_variable = disp_x
    component = x
    use_displaced_mesh = true
    compute_lm_residuals = false
    correct_edge_dropping = true
    weighted_gap_uo = weighted_gap_uo
  []
  [normal_y]
    type = NormalMortarMechanicalContact
    primary_boundary = '23'
    secondary_boundary = '11'
    primary_subdomain = 'primary_lower'
    secondary_subdomain = 'secondary_lower'
    variable = normal_lm
    secondary_variable = disp_y
    component = y
    use_displaced_mesh = true
    compute_lm_residuals = false
    correct_edge_dropping = true
    weighted_gap_uo = weighted_gap_uo
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
  solve_type = 'NEWTON'

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type -pc_factor_shift_type '
                        '-pc_factor_shift_amount'
  petsc_options_value = 'lu    superlu_dist nonzero 1e-10'

  line_search = 'none'

  dt = 0.1
  dtmin = 0.01
  end_time = 0.5

  l_max_its = 20

  nl_max_its = 20
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-9
[]

[Outputs]
  csv = true
  execute_on = 'FINAL'
[]

[Postprocessors]
  [contact]
    type = ContactDOFSetSize
    variable = normal_lm
    subdomain = 'secondary_lower'
  []
  [normal_lm]
    type = ElementAverageValue
    variable = normal_lm
    block = 'secondary_lower'
  []
  [avg_disp_x]
    type = ElementAverageValue
    variable = disp_x
    block = '1 2'
  []
  [avg_disp_y]
    type = ElementAverageValue
    variable = disp_y
    block = '1 2'
  []
  [max_disp_x]
    type = ElementExtremeValue
    variable = disp_x
    block = '1 2'
  []
  [min_disp_x]
    type = ElementExtremeValue
    variable = disp_x
    block = '1 2'
    value_type = min
  []
[]
