# Framework-level companion to modules/contact/test/tests/mortar_late_activation/mortar_late_activation.i.
# That test only exercises ComputeMortarFunctor through the contact module's [Contact] action; this one
# drives the same code path with a plain framework [Constraints] block, so the fix has coverage that
# does not depend on the contact module.
#
# SETUP/LOADING: identical geometry and timing to mortar_late_activation.i and
# nodal_aux_late_activation.i - the right block starts offset in y so every mortar segment is pruned at
# initialization (amg.secondaryIPSubIDs()/primaryIPSubIDs() empty, so
# ComputeMortarFunctor::_secondary_ip_sub_to_mats/_primary_ip_sub_to_mats have no entry for either
# interior-parent subdomain), then slides down via a prescribed disp_y AuxVariable until segments appear
# at t ~ 0.6.
#
# EXPECTED: unpatched MOOSE throws once the first segment appears; patched MOOSE runs to end_time.

[GlobalParams]
  use_displaced_mesh = true
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [left_block]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -1.0
    xmax = 0.0
    ymin = -0.5
    ymax = 0.5
    nx = 2
    ny = 3
    elem_type = QUAD4
    boundary_name_prefix = lb
  []
  [left_block_id]
    type = SubdomainIDGenerator
    input = left_block
    subdomain_id = 1
  []

  # Offset up in y => no lateral overlap with lb at t=0; permanent 0.1 gap in x, same as
  # mortar_late_activation.i.
  [right_block]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0.1
    xmax = 1.1
    ymin = 2.0
    ymax = 3.2
    nx = 2
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
  [primary]
    type = LowerDBlockFromSidesetGenerator
    input = combined
    sidesets = 'rb_left'
    new_block_id = 20
  []
  [secondary]
    type = LowerDBlockFromSidesetGenerator
    input = primary
    sidesets = 'lb_right'
    new_block_id = 10
  []
[]

[Functions]
  # Right block spans y in [2 - 2.5t, 3.2 - 2.5t]; overlap with the secondary face
  # (y in [-0.5, 0.5]) begins once 2 - 2.5t < 0.5, i.e. t > 0.6.
  [slide_down]
    type = ParsedFunction
    expression = '-2.5 * t'
  []
[]

[AuxVariables]
  [disp_x]
  []
  [disp_y]
    [AuxKernel]
      type = FunctionAux
      function = slide_down
      block = '2'
      execute_on = 'INITIAL TIMESTEP_BEGIN'
    []
  []
[]

[Variables]
  [T]
    block = '1 2'
  []
  [lambda]
    block = '10'
    use_dual = true
  []
[]

[Kernels]
  [conduction]
    type = Diffusion
    variable = T
    block = '1 2'
  []
[]

[BCs]
  [lb_fix]
    type = DirichletBC
    variable = T
    boundary = lb_left
    value = 0
  []
  [rb_fix]
    type = DirichletBC
    variable = T
    boundary = rb_right
    value = 1
  []
[]

[Constraints]
  [mortar]
    type = EqualValueConstraint
    variable = lambda
    secondary_variable = T
    primary_boundary = rb_left
    secondary_boundary = lb_right
    primary_subdomain = 20
    secondary_subdomain = 10
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Problem]
  # The mortar Jacobian coupling between lambda and T only exists once a segment survives
  # projection (t > 0.6), so the sparsity pattern computed at t=0 necessarily grows partway
  # through the solve. MooseTestApp errors on Jacobian nonzero reallocation by default (unlike
  # the CombinedApp used by mortar_late_activation.i), so this has to be relaxed explicitly here.
  error_on_jacobian_nonzero_reallocation = false
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  # Until t = 1.0 only part of the secondary face is covered by a mortar segment, so lambda is
  # singular on the uncovered part (zero row and column); the shift below is what makes that
  # part of the system factorizable at all, not a convergence nicety.
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = 'lu       NONZERO               1e-15'
  dt = 0.1
  dtmin = 0.1
  end_time = 1.0
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-10
[]

[Postprocessors]
  # The mortar constraint's observable effect: with no surviving segments the two blocks are
  # decoupled, so T is 0 on the secondary face and 1 on the primary face; once segments appear
  # the weak equality pulls them together. Deliberately NOT postprocessing lambda: until t = 1.0
  # only part of the secondary face is covered by segments, and the lambda nodes outside that
  # part get their value from -pc_factor_shift_amount above rather than from the solve, so they
  # are neither physical nor partition-reproducible.
  [T_secondary]
    type = SideAverageValue
    variable = T
    boundary = lb_right
  []
  [T_primary]
    type = SideAverageValue
    variable = T
    boundary = rb_left
  []
[]

[Outputs]
  csv = true
[]
