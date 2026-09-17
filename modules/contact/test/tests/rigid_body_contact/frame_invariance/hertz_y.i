# Frame-invariance regression, Y-direction contact.
#
# Baseline: identical to `load_controlled_elastic_3d/hertz_load_3d.i`
# (proven-convergent plain-Newton load-control setup).  Companion inputs
# `hertz_x.i` and `hertz_z.i` build the SAME physical problem rotated by
# 90 degrees onto the X and Z axes (via TransformGenerator).  Every
# solution invariant recorded in the CSV -- `applied_force`,
# `indenter_translation`, `max_lm`, `cumulative_nl` -- should match all
# three inputs bit-for-bit up to floating-point roundoff.  Any
# axis-dependent bug in SphereContactor / LevelSetContactor /
# RigidBodyLoadControl / RigidBodyNormalMechanicalContact /
# RigidBodyNodalNCPKernel / RigidBodyContactSparsity breaks at least one
# of them.
#
# We chose end_time = 0.1 (one time step) to keep the test light: three
# 3D contact solves under CI in a few seconds each.  The physics is
# already fully exercised at t = 0.1 -- LM active-set determined,
# scalar-row Jacobian populated, PETSc contact Jacobian solved.

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  large_kinematics = false
[]

[Mesh]
  [file]
    type = FileMeshGenerator
    file = ../../hertz_spherical/hertz_contact.e
  []
  [drop_rigid_indenter]
    type = BlockDeletionGenerator
    input = file
    block = 1000
  []
  [contact_lower]
    type = LowerDBlockFromSidesetGenerator
    input = drop_rigid_indenter
    sidesets = '100'
    new_block_id = 10001
    new_block_name = contact_lower
  []
  allow_renumbering = false
[]

[UserObjects]
  [contact_sparsity]
    type = RigidBodyContactSparsity
    lm_variable = normal_lm
    displacements = 'disp_x disp_y disp_z'
    boundary = 100
  []
  [sphere]
    type = SphereContactor
    center = '0 -4 0'
    radius = 2.0
    disp_y_scalar = indenter_y
  []
  [nodal_area]
    type = NodalArea
    boundary = 100
    variable = nodal_area
    execute_on = 'INITIAL LINEAR'
  []
[]

[Functions]
  [applied_force]
    type = PiecewiseLinear
    x = '0 1'
    y = '0 1.0e5'
  []
[]

[Variables]
  [disp_x]
    block = '1 contact_lower'
  []
  [disp_y]
    block = '1 contact_lower'
  []
  [disp_z]
    block = '1 contact_lower'
  []
  [normal_lm]
    block = contact_lower
  []
  [indenter_y]
    family = SCALAR
    order = FIRST
    initial_condition = 0.005
  []
[]

[AuxVariables]
  [nodal_area]
    family = LAGRANGE
    order = FIRST
  []
[]

[Kernels]
  [sdx]
    type = TotalLagrangianStressDivergence
    variable = disp_x
    component = 0
    block = 1
  []
  [sdy]
    type = TotalLagrangianStressDivergence
    variable = disp_y
    component = 1
    block = 1
  []
  [sdz]
    type = TotalLagrangianStressDivergence
    variable = disp_z
    component = 2
    block = 1
  []
[]

[NodalKernels]
  [ncp]
    type = RigidBodyNodalNCPKernel
    variable = normal_lm
    contactor = sphere
    displacements = 'disp_x disp_y disp_z'
    block = contact_lower
    c = 1.0
  []
[]

[ScalarKernels]
  [load_control]
    type = RigidBodyLoadControl
    variable = indenter_y
    boundary = 100
    force = applied_force
    contactor = sphere
    nodal_area = nodal_area
    lm_variable = normal_lm
    displacements = 'disp_x disp_y disp_z'
    direction = '0 1 0'
    c = 1.0
  []
[]

[Materials]
  [tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1.40625e7
    poissons_ratio = 0.25
    block = 1
  []
  [stress]
    type = ComputeLagrangianLinearElasticStress
    block = 1
  []
  [strain]
    type = ComputeLagrangianStrain
    block = 1
  []
[]

[BCs]
  [rb_tx]
    type = RigidBodyNormalMechanicalContact
    variable = disp_x
    lowerd_variable = normal_lm
    boundary = 100
    contactor = sphere
    component = x
    displacements = 'disp_x disp_y disp_z'
  []
  [rb_ty]
    type = RigidBodyNormalMechanicalContact
    variable = disp_y
    lowerd_variable = normal_lm
    boundary = 100
    contactor = sphere
    component = y
    displacements = 'disp_x disp_y disp_z'
  []
  [rb_tz]
    type = RigidBodyNormalMechanicalContact
    variable = disp_z
    lowerd_variable = normal_lm
    boundary = 100
    contactor = sphere
    component = z
    displacements = 'disp_x disp_y disp_z'
  []
  [symm_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  []
  [symm_z]
    type = DirichletBC
    variable = disp_z
    boundary = 3
    value = 0.0
  []
  [pin_top]
    type = DirichletBC
    variable = disp_y
    boundary = 2
    value = 0.0
  []
[]

[Problem]
  kernel_coverage_check = false
  material_coverage_check = false
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  automatic_scaling = true

  petsc_options_iname = '-snes_type -pc_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = 'newtonls    lu       NONZERO               1e-12'
  line_search = basic

  nl_rel_tol = 1e-9
  nl_abs_tol = 1e-8
  nl_max_its = 40
  l_max_its = 200

  start_time = 0.0
  end_time   = 0.1
  dt = 0.05
[]

[Postprocessors]
  # These five quantities are frame-invariant.  The three axis-variant
  # inputs must all reproduce them.
  [applied_force]
    type = FunctionValuePostprocessor
    function = applied_force
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [indenter_translation]
    type = ScalarVariable
    variable = indenter_y
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [max_lm]
    type = NodalExtremeValue
    variable = normal_lm
    block = contact_lower
    value_type = max
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [num_nl]
    type = NumNonlinearIterations
  []
  [cumulative_nl]
    type = CumulativeValuePostprocessor
    postprocessor = num_nl
  []
[]

[Outputs]
  [csv]
    type = CSV
    execute_on = 'TIMESTEP_END'
  []
[]
