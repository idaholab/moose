# Example: rigid sphere pressed into a J2-plasticity body, 3D
# quarter-symmetry, LARGE DEFORMATION.
#
# Same mesh + rigid-indenter treatment as the elastic 3D example, but the
# deformable body is now finite-strain J2 plasticity (linear hardening) and
# the load is ramped further to activate a large plastic zone.
#
# Contact stack: analytic level-set via `[RigidContact]` action.  Setting
# `force = ...` activates the load-control extensions.  Uses
# `SurfaceMeshContactor` from an STL geometry (unit sphere scaled).
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
#
# Solver: this example uses `UzawaTransient`, which splits the coupled
# (u, lambda, s) system into an outer 1D Newton on `s` wrapping an
# inner primal solve on (u, lambda).  See `uzawa_solver_plan.md`.

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [file]
    type = FileMeshGenerator
    file = contact_test.e
  []
  [drop_rigid_indenter]
    type = BlockDeletionGenerator
    input = file
    block = 1
  []
  allow_renumbering = false
[]

[Variables]
  [disp_x]
    block = '1000 contact_lower'
  []
  [disp_y]
    block = '1000 contact_lower'
  []
  [disp_z]
    block = '1000 contact_lower'
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    strain = FINITE
    add_variables = false
    compatibility_mode = true
    decomposition_method = EigenSolution
    volumetric_locking_correction = true
    block = 1000
  []
[]

[UserObjects]
  [sphere]
    type = SurfaceMeshContactor
    file = unit_sphere.stl
    scale = 2.0
    disp_y_scalar = indenter_y
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

[RigidContact]
  [top]
    contactor = sphere
    boundary  = mat_top
    displacements = 'disp_x disp_y disp_z'
    lm_variable_name   = normal_lm
    lower_d_block_name = contact_lower
    # Plain-Newton inner solve -- no SSLS + bounds needed.
    enforce_bounds = false
    # Force-control extensions:
    force          = applied_force
    load_direction = '0 -1 0'
    scalar_variable_name = indenter_y
    nodal_area_variable_name = nodal_area
    # `kss_stiffness` acts as UzawaTransient's outer scalar-Newton
    # dR_s/ds approximation.  1e6 is a good compromise between the
    # elastic Hertz tangent (~1e7 at first contact) and the much softer
    # plastic response later in the ramp.
    kss_stiffness = 1e6
  []
[]

[AuxVariables]
  [plastic_strain_mag]
    order = CONSTANT
    family = MONOMIAL
    block = 1000
  []
  [stress_xx]
    order = CONSTANT
    family = MONOMIAL
    block = 1000
  []
  [stress_yy]
    order = CONSTANT
    family = MONOMIAL
    block = 1000
  []
  [stress_zz]
    order = CONSTANT
    family = MONOMIAL
    block = 1000
  []
  [stress_xy]
    order = CONSTANT
    family = MONOMIAL
    block = 1000
  []
  [stress_xz]
    order = CONSTANT
    family = MONOMIAL
    block = 1000
  []
  [stress_yz]
    order = CONSTANT
    family = MONOMIAL
    block = 1000
  []
[]

[AuxKernels]
  [plastic_strain_mag]
    type = MaterialRealAux
    property = eff_plastic_strain
    variable = plastic_strain_mag
    execute_on = 'TIMESTEP_END'
    block = 1000
  []
  [stress_xx]
    type = RankTwoAux
    rank_two_tensor = cauchy_stress
    variable = stress_xx
    index_i = 0
    index_j = 0
    execute_on = 'TIMESTEP_END'
    block = 1000
  []
  [stress_yy]
    type = RankTwoAux
    rank_two_tensor = cauchy_stress
    variable = stress_yy
    index_i = 1
    index_j = 1
    execute_on = 'TIMESTEP_END'
    block = 1000
  []
  [stress_zz]
    type = RankTwoAux
    rank_two_tensor = cauchy_stress
    variable = stress_zz
    index_i = 2
    index_j = 2
    execute_on = 'TIMESTEP_END'
    block = 1000
  []
  [stress_xy]
    type = RankTwoAux
    rank_two_tensor = cauchy_stress
    variable = stress_xy
    index_i = 0
    index_j = 1
    execute_on = 'TIMESTEP_END'
    block = 1000
  []
  [stress_xz]
    type = RankTwoAux
    rank_two_tensor = cauchy_stress
    variable = stress_xz
    index_i = 0
    index_j = 2
    execute_on = 'TIMESTEP_END'
    block = 1000
  []
  [stress_yz]
    type = RankTwoAux
    rank_two_tensor = cauchy_stress
    variable = stress_yz
    index_i = 1
    index_j = 2
    execute_on = 'TIMESTEP_END'
    block = 1000
  []
[]

[Materials]
  [tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1.40625e7
    poissons_ratio = 0.25
    block = 1000
  []
  # `stress` published here; Physics action auto-wraps with
  # ComputeLagrangianWrappedStress and creates the strain material.
  [stress]
    type = ComputeMultiPlasticityStress
    plastic_models = j2
    ep_plastic_tolerance = 1e-9
    block = 1000
  []
  [eff_plastic_strain]
    type = RankTwoInvariant
    rank_two_tensor = plastic_strain
    property_name = eff_plastic_strain
    invariant = EffectiveStrain
    block = 1000
  []
[]

[Functions]
  [applied_force]
    # Ramp to a target reaction that induces significant plasticity in
    # the material.  At F(1) = 1.5e5 the plastic zone extends beyond
    # the contact patch.
    type = PiecewiseLinear
    x = '0 1'
    y = '0 1.5e5'
  []
[]

[BCs]
  [symm_x]
    type = DirichletBC
    variable = disp_x
    boundary = mat_sym_x
    value = 0.0
  []
  [symm_z]
    type = DirichletBC
    variable = disp_z
    boundary = mat_sym_z
    value = 0.0
  []
  [pin_top]
    type = DirichletBC
    variable = disp_y
    boundary = mat_bot
    value = 0.0
  []
[]

[Executioner]
  type = UzawaTransient

  # Outer scalar Newton knobs.
  load_control_kernel = rigid_contact_load_control_top
  outer_max_iter      = 200
  outer_abs_tol       = 10
  outer_rel_tol       = 1e-3
  max_step            = 5e-3

  # Inner primal solve: plain Newton + LU.  Auto-scaling off so the
  # kss_stiffness shift on the scalar row is not renormalized away.
  solve_type = NEWTON
  petsc_options_iname = '-snes_type -pc_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = 'newtonls    lu       NONZERO               1e-12'
  line_search = basic
  automatic_scaling = false

  nl_rel_tol = 1e-9
  nl_abs_tol = 1e-8
  nl_max_its = 40
  l_max_its = 200

  start_time = 0.0
  end_time   = 1.0

  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 0.005
    growth_factor = 1.5
    cutback_factor = 0.5
    optimal_iterations = 8
    iteration_window = 2
  []

  # Predictor: staged for the future; currently does not rescue this
  # hard plastic case (see `uzawa_npc_plan.md`).  UzawaTransient itself
  # drives convergence.
  [Predictor]
    type = RigidBodyContactPredictor
    boundary = mat_top
    lm_variable = normal_lm
    displacements = 'disp_x disp_y disp_z'
    scalar_variable = indenter_y
    k_hops = 3
    sub_max_iter = 20
  []
[]

[Postprocessors]
  [max_lm]
    type = NodalExtremeValue
    variable = normal_lm
    block = contact_lower
    value_type = max
  []
  [max_plastic_strain]
    type = ElementExtremeValue
    variable = plastic_strain_mag
    block = 1000
    value_type = max
  []
  [num_nl]
    type = NumNonlinearIterations
  []
  [cumulative_nl]
    type = CumulativeValuePostprocessor
    postprocessor = num_nl
  []
  # Force-controlled: the total contactor force IS the applied target
  # F(t), by definition of the load-control constraint.
  [contactor_force]
    type = FunctionValuePostprocessor
    function = applied_force
  []
  # Signed contactor position along the axis whose translation this
  # scalar drives; load_direction is -y, so as the contactor descends
  # into the material `indenter_y` decreases from 0.
  [contactor_displacement]
    type = ScalarVariable
    variable = indenter_y
  []
[]

[Outputs]
  exodus = true
  csv = true
[]
