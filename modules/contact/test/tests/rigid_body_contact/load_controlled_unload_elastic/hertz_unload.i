# Load-controlled Hertz test, 2D-axisym elastic -- full LOAD then UNLOAD cycle.
#
# Same physics and load-control setup as `load_controlled_elastic/hertz_load.i`,
# but F(t) is a triangle wave: 0 -> 1e4 (t = 0 -> 1) -> 0 (t = 1 -> 2).  On
# unloading, Newton on the coupled (u, lambda, s) system must smoothly
# reduce lambda_i -> 0 as each contact node disengages and drive s back
# toward the point of first contact.
#
# Solver choice: plain `newtonls` (no bounds).  Complementarity is closed
# by physics -- the material can't pull on the rigid body across the
# contact interface -- so lambda remains >= 0 on its own.  Empirically
# this is the fastest AND most robust option for load-controlled contact
# with unloading:
#
#   solver                          | wall time | notes
#   plain newtonls (this test)      |    ~1 s   | 10 time steps, clean cutbacks
#   vinewtonssls + basic + bounds   |   ~16 s   | frequent cutbacks near F(t) values
#                                                 that fall between mesh-quantum
#                                                 active-set states.
#
# The mesh-quantum caveat: on this coarse 2D mesh (~10 LM nodes on the
# contact sideset), individual node engagement/disengagement changes the
# integrated reaction by a value comparable to intermediate F(t) targets,
# so any strategy that decouples the (u, lambda) solve from the scalar
# update (Uzawa-style) can oscillate through active-set boundaries and
# fail.  Monolithic Newton absorbs these transitions smoothly.

[GlobalParams]
  displacements = 'disp_x disp_y'
  large_kinematics = false
[]

[Mesh]
  [file]
    type = FileMeshGenerator
    file = ../../hertz_spherical/hertz_contact_rz.e
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
  coord_type = RZ
  allow_renumbering = false
[]

[UserObjects]
  [contact_sparsity]
    type = RigidBodyContactSparsity
    lm_variable = normal_lm
    displacements = 'disp_x disp_y'
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
    # Triangle wave: load up to 1e4 at t = 1, then release back to 0 at t = 2.
    type = PiecewiseLinear
    x = '0 1     2'
    y = '0 1.0e4 0'
  []
[]

[Variables]
  [disp_x]
    block = '1 contact_lower'
  []
  [disp_y]
    block = '1 contact_lower'
  []
  [normal_lm]
    block = contact_lower
  []
  [indenter_y]
    family = SCALAR
    order = FIRST
    # Seed with a small positive value so several LM nodes are on the gap
    # branch of min(lambda, c*g) at t = 0.  Without this seed the contactor
    # is exactly tangent to the material tip and every candidate LM node
    # sits on the min-NCP kink, leaving Newton no direction on the first
    # step.  This is a standard complementarity-solver technique, not a
    # fictitious stiffness.
    initial_condition = 0.01
  []
[]

[AuxVariables]
  [bounds_dummy]
    family = LAGRANGE
    order = FIRST
    block = contact_lower
  []
  [nodal_area]
    family = LAGRANGE
    order = FIRST
  []
[]

[Bounds]
  # Inactive by default -- see header comment.  Plain Newton without
  # bounds is the shipped default because vinewtonssls + bounds
  # cutback-thrashes on unload when a target force falls between
  # mesh-quantum active-set states.
  active = ''
  [lm_lo]
    type = ConstantBounds
    variable = bounds_dummy
    bounded_variable = normal_lm
    bound_type = lower
    bound_value = 0.0
  []
  [lm_hi]
    type = ConstantBounds
    variable = bounds_dummy
    bounded_variable = normal_lm
    bound_type = upper
    bound_value = 1e12
  []
[]

[Kernels]
  [sdx]
    type = TotalLagrangianStressDivergenceAxisymmetricCylindrical
    variable = disp_x
    component = 0
    block = 1
  []
  [sdy]
    type = TotalLagrangianStressDivergenceAxisymmetricCylindrical
    variable = disp_y
    component = 1
    block = 1
  []
[]

[NodalKernels]
  [ncp]
    type = RigidBodyNodalNCPKernel
    variable = normal_lm
    contactor = sphere
    displacements = 'disp_x disp_y'
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
    displacements = 'disp_x disp_y'
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
    type = ComputeLagrangianStrainAxisymmetricCylindrical
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
    displacements = 'disp_x disp_y'
  []
  [rb_ty]
    type = RigidBodyNormalMechanicalContact
    variable = disp_y
    lowerd_variable = normal_lm
    boundary = 100
    contactor = sphere
    component = y
    displacements = 'disp_x disp_y'
  []
  [symm_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1
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
  petsc_options_value = 'newtonls    lu     NONZERO               1e-12'

  line_search = basic

  nl_rel_tol = 1e-9
  nl_abs_tol = 1e-8
  nl_max_its = 40
  l_max_its = 200

  start_time = 0.0
  end_time   = 2.0

  # Adaptive dt: complementarity + scalar coupling has a small local
  # convergence radius when the applied force jumps quickly.  Start small
  # and grow as Newton stays fast.
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 0.005
    growth_factor = 2.0
    cutback_factor = 0.5
    optimal_iterations = 8
    iteration_window = 3
  []
[]

[Postprocessors]
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
