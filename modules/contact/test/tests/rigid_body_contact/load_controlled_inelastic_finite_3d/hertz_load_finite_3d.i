# Example: rigid sphere pressed into a J2-plasticity body, 3D
# quarter-symmetry, LARGE DEFORMATION.
#
# Same mesh + rigid-indenter treatment as the elastic 3D example, but the
# deformable body is now finite-strain J2 plasticity (linear hardening) and
# the load is ramped further to activate a large plastic zone.
#
# Contact stack: analytic level-set (SphereContactor + RigidBodyNodalNCPKernel
# + RigidBodyNormalMechanicalContact).  Per-node min-NCP, no mortar, no AD,
# no dual basis.  Runs much faster and to a much larger indentation than the
# mortar path could sustain (which was AD-container-limited).
#
# Constitutive stack (new-Lagrangian pipeline with consistent algorithmic
# tangent):
#   ComputeLagrangianStrain (kinematic_approximation = rashid_eigen)
#     + ComputeLagrangianWrappedStress (objective_rate = rashid)
#     + ComputeMultiPlasticityStress (plastic_models = j2)
#     + SolidMechanicsPlasticJ2 + SolidMechanicsHardeningPowerRule
#   TotalLagrangianStressDivergence with large_kinematics = true
#   stabilize_strain = true to avoid nearly-incompressible plastic locking
#     on linear hexes.

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  large_kinematics = true
  stabilize_strain = true
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
  [yield_strength]
    type = SolidMechanicsHardeningPowerRule
    value_0 = 2.0e5
    epsilon0 = 0.2
    exponent = 1.0
  []
  [j2]
    type = SolidMechanicsPlasticJ2
    yield_strength = yield_strength
    yield_function_tolerance = 1e-3
    internal_constraint_tolerance = 1e-9
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
  [bounds_dummy]
    family = LAGRANGE
    order = FIRST
    block = contact_lower
  []
  [nodal_area]
    family = LAGRANGE
    order = FIRST
  []
  [plastic_strain_mag]
    order = CONSTANT
    family = MONOMIAL
    block = 1
  []
[]

[Bounds]
  # Inactive by default (plain Newton is what converges).  Enabled via
  # cli_args by the -jac diagnostic variants.
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

[AuxKernels]
  [plastic_strain_mag]
    type = MaterialRealAux
    property = eff_plastic_strain
    variable = plastic_strain_mag
    execute_on = 'TIMESTEP_END'
    block = 1
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
    type = ComputeLagrangianWrappedStress
    objective_rate = rashid
    block = 1
  []
  [wrapped]
    type = ComputeMultiPlasticityStress
    plastic_models = j2
    ep_plastic_tolerance = 1e-9
    block = 1
  []
  [strain]
    type = ComputeLagrangianStrain
    kinematic_approximation = rashid_eigen
    block = 1
  []
  [eff_plastic_strain]
    type = RankTwoInvariant
    rank_two_tensor = plastic_strain
    property_name = eff_plastic_strain
    invariant = EffectiveStrain
    block = 1
  []
[]

[Functions]
  [applied_force]
    # Regression-test ramp: F(1) = 5e4 induces yield locally without
    # driving the material into heavy plasticity, so the run completes
    # in reasonable CI time.  The corresponding standalone example
    # (examples/rigid/ld-inelastic-force/) ramps to 1.5e5 for a fuller
    # plastic zone.
    type = PiecewiseLinear
    x = '0 1'
    y = '0 5.0e4'
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
    finite_strain = true
    displacements = 'disp_x disp_y disp_z'
  []
  [rb_ty]
    type = RigidBodyNormalMechanicalContact
    variable = disp_y
    lowerd_variable = normal_lm
    boundary = 100
    contactor = sphere
    component = y
    finite_strain = true
    displacements = 'disp_x disp_y disp_z'
  []
  [rb_tz]
    type = RigidBodyNormalMechanicalContact
    variable = disp_z
    lowerd_variable = normal_lm
    boundary = 100
    contactor = sphere
    component = z
    finite_strain = true
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

  # Plain Newton with LU direct solve (see the elastic 3D force-control
  # test for the reason we can't use SSLS + semismooth line search here).
  petsc_options_iname = '-snes_type -pc_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = 'newtonls    lu       NONZERO               1e-12'

  line_search = basic

  nl_rel_tol = 1e-9
  nl_abs_tol = 1e-8
  nl_max_its = 40
  l_max_its = 200

  start_time = 0.0
  end_time   = 1.0

  # Adaptive time stepping: analytic-level-set contact concentrates load at a
  # single node initially, so start small and grow.  Rashid-eigen strain
  # increments can fail (non-symmetric tensor) if a single Newton step
  # over-shoots the elastic-plastic corner.
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 0.005
    growth_factor = 1.5
    cutback_factor = 0.5
    optimal_iterations = 8
    iteration_window = 2
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
  [max_plastic_strain]
    type = ElementExtremeValue
    variable = plastic_strain_mag
    block = 1
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
