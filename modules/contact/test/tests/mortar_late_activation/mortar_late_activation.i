# MOOSE bug reproducer: a mortar interface with NO mortar segments at initialization throws
# map_find() the instant its first segment appears.
#
# Derived from modules/contact/test/tests/mortar_tm/horizontal_blocks_mortar_TM.i, changed only in
# the geometry/loading needed to exhibit the bug.
#
# SETUP: two blocks with a permanent 0.1 x-gap, offset in y so the facing sidesets do NOT overlap
# laterally at t=0. Every mortar segment is therefore pruned at initialization
# (AutomaticMortarGeneration.C:1082, |xi2| > 1+TOLERANCE), so amg.secondaryIPSubIDs() is EMPTY and
# Moose::Mortar::setupMortarMaterials inserts NO key for the secondary interior-parent subdomain.
#
# LOADING: the right block slides DOWN into lateral alignment. Overlap begins at t ~ 0.6. As soon as
# one segment survives projection, MortarUtils.h:144 does
#     libmesh_map_find(secondary_ip_sub_to_mats, secondary_ip->subdomain_id())
# on a map with no such key -> throw:
#     map_find() error: key "1" not found in .../MortarUtils.h on line 144
#
# The blocks NEVER touch (the x-gap stays 0.1), so the Lagrange multiplier stays zero throughout.
# The trigger is PROJECTION coming into range, not contact engaging.
#
# EXPECTED: unpatched MOOSE throws at t ~ 0.6; patched MOOSE runs to end_time.

[GlobalParams]
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = true
[]

[Mesh]
  [left_block]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -1.0
    xmax = 0.0
    ymin = -0.5
    ymax = 0.5
    nx = 1
    ny = 3
    elem_type = QUAD4
    boundary_name_prefix = lb
  []
  [left_block_id]
    type = SubdomainIDGenerator
    input = left_block
    subdomain_id = 1
  []

  # Offset UP in y by 2.5 => no lateral overlap with lb_right at t=0, and a 0.1 gap in x.
  [right_block]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0.1
    xmax = 1.1
    ymin = 2.0
    ymax = 3.2
    nx = 1
    ny = 3
    elem_type = QUAD4
    boundary_name_prefix = rb
    boundary_id_offset = 10
  []
  [right_block_id]
    type = SubdomainIDGenerator
    input = right_block
    subdomain_id = 2
  []

  [combined]
    type = MeshCollectionGenerator
    inputs = 'left_block_id right_block_id'
  []
  [block_rename]
    type = RenameBlockGenerator
    input = combined
    old_block = '1 2'
    new_block = 'left_block right_block'
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    strain = FINITE
    incremental = true
    add_variables = true
    block = '1 2'
  []
[]

[Functions]
  # Slide the right block down. rb spans y in [2 - 2.5t, 3.2 - 2.5t]; overlap with the secondary
  # face (y in [-0.5, 0.5]) begins once 2 - 2.5t < 0.5, i.e. t > 0.6.
  [slide_down]
    type = ParsedFunction
    expression = '-2.5 * t'
  []
[]

[BCs]
  [lb_fix_x]
    type = DirichletBC
    preset = true
    variable = disp_x
    boundary = lb_left
    value = 0.0
  []
  [lb_fix_y]
    type = DirichletBC
    preset = true
    variable = disp_y
    boundary = lb_left
    value = 0.0
  []
  [rb_fix_x]
    type = DirichletBC
    preset = true
    variable = disp_x
    boundary = rb_right
    value = 0.0
  []
  [rb_slide_y]
    type = FunctionDirichletBC
    preset = true
    variable = disp_y
    boundary = rb_right
    function = slide_down
  []
[]

[Materials]
  [elasticity_tensor_left]
    type = ComputeIsotropicElasticityTensor
    block = left_block
    youngs_modulus = 1.0e6
    poissons_ratio = 0.3
  []
  [stress_left]
    type = ComputeFiniteStrainElasticStress
    block = left_block
  []
  [elasticity_tensor_right]
    type = ComputeIsotropicElasticityTensor
    block = right_block
    youngs_modulus = 1.0e6
    poissons_ratio = 0.3
  []
  [stress_right]
    type = ComputeFiniteStrainElasticStress
    block = right_block
  []
[]

[Contact]
  [leftright]
    secondary = lb_right
    primary = rb_left
    model = frictionless
    formulation = mortar
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -mat_mffd_err -pc_factor_shift_type -pc_factor_shift_amount -snes_max_it'
  petsc_options_value = 'lu       1e-5          NONZERO               1e-15                   20'
  dt = 0.1
  dtmin = 0.1
  end_time = 1.0
  l_tol = 1e-4
  l_max_its = 100
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-6
  nl_max_its = 100
[]

[Postprocessors]
  # The multiplier stays at exactly zero for the whole run because the blocks never touch: the
  # 0.1 gap normal to the interface is held by the disp_x BCs on lb_left and rb_right. This is what
  # shows the bug is triggered by projection coming into range rather than by contact engaging.
  [max_normal_lm]
    type = NodalExtremeValue
    variable = leftright_normal_lm
    value_type = max
    block = leftright_secondary_subdomain
  []
  [min_normal_lm]
    type = NodalExtremeValue
    variable = leftright_normal_lm
    value_type = min
    block = leftright_secondary_subdomain
  []
[]

[Outputs]
  csv = true
[]
