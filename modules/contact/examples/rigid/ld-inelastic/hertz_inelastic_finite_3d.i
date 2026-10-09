# Example: rigid sphere pressed into a J2-plasticity body, 3D
# quarter-symmetry, LARGE DEFORMATION.
#
# Same mesh + rigid-indenter treatment as the elastic 3D example, but the
# deformable body is now finite-strain J2 plasticity (linear hardening) and
# the load is ramped further to activate a large plastic zone.
#
# Contact stack: analytic level-set via `[RigidContact]` action
# (SphereContactor + expanded RigidBodyNodalNCPKernel +
# RigidBodyNormalMechanicalContact + bounds + sparsity + preconditioning).
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

# Top-level mesh-resolution knob; passed to `nr` on SphereMeshGenerator.
# See the elastic example for the accuracy vs runtime trade-off.
sphere_refinement = 3

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [sphere]
    type = SphereMeshGenerator
    radius = 2.0
    nr = ${sphere_refinement}
  []
  [cut_upper_half]
    type = PlaneDeletionGenerator
    input = sphere
    point = '0 0 0'
    normal = '0 1 0'
  []
  [cut_neg_x]
    type = PlaneDeletionGenerator
    input = cut_upper_half
    point = '0 0 0'
    normal = '-1 0 0'
  []
  [cut_neg_z]
    type = PlaneDeletionGenerator
    input = cut_neg_x
    point = '0 0 0'
    normal = '0 0 -1'
  []
  [assign_block]
    type = RenameBlockGenerator
    input = cut_neg_z
    old_block = 0
    new_block = 1
  []
  [rename_curved_ss]
    type = RenameBoundaryGenerator
    input = assign_block
    old_boundary = 0
    new_boundary = 100
  []
  [flat_sidesets]
    type = SideSetsFromNormalsGenerator
    input = rename_curved_ss
    normals = '-1  0  0
                0  1  0
                0  0 -1'
    new_boundary = '1 2 3'
    normal_tol = 1e-6
    fixed_normal = true
  []
  allow_renumbering = false
[]

[Variables]
  # Disp variables span both block 1 and the lower-d block so
  # RigidContact's NCP kernel and BCs find them via block subset.
  # SolidMechanics/QuasiStatic below adds kernels + strain material +
  # wrapping stress on block 1 only.
  [disp_x]
    block = '1 contact_lower'
  []
  [disp_y]
    block = '1 contact_lower'
  []
  [disp_z]
    block = '1 contact_lower'
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    strain = FINITE
    add_variables = false
    compatibility_mode = true          # auto-wraps user's `stress` via ComputeLagrangianWrappedStress
    decomposition_method = EigenSolution  # -> kinematic_approximation = rashid_eigen on ComputeLagrangianStrain
    volumetric_locking_correction = true  # -> stabilize_strain = true
    block = 1
    # Assembled residual per component saved to aux vars for the
    # `contactor_force` reaction-sum postprocessor below.
    save_in = 'saved_x saved_y saved_z'
  []
[]

[UserObjects]
  [sphere]
    type = SphereContactor
    center = '0 -4 0'
    radius = 2.0
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
    boundary  = 100
    displacements = 'disp_x disp_y disp_z'
    lm_variable_name   = normal_lm
    lower_d_block_name = contact_lower
  []
[]

[AuxVariables]
  # Residual-save targets for the `contactor_force` reaction sum below.
  [saved_x]
    block = 1
  []
  [saved_y]
    block = 1
  []
  [saved_z]
    block = 1
  []
  [plastic_strain_mag]
    order = CONSTANT
    family = MONOMIAL
    block = 1
  []
  [stress_xx]
    order = CONSTANT
    family = MONOMIAL
    block = 1
  []
  [stress_yy]
    order = CONSTANT
    family = MONOMIAL
    block = 1
  []
  [stress_zz]
    order = CONSTANT
    family = MONOMIAL
    block = 1
  []
  [stress_xy]
    order = CONSTANT
    family = MONOMIAL
    block = 1
  []
  [stress_xz]
    order = CONSTANT
    family = MONOMIAL
    block = 1
  []
  [stress_yz]
    order = CONSTANT
    family = MONOMIAL
    block = 1
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
  [stress_xx]
    type = RankTwoAux
    rank_two_tensor = cauchy_stress
    variable = stress_xx
    index_i = 0
    index_j = 0
    execute_on = 'TIMESTEP_END'
    block = 1
  []
  [stress_yy]
    type = RankTwoAux
    rank_two_tensor = cauchy_stress
    variable = stress_yy
    index_i = 1
    index_j = 1
    execute_on = 'TIMESTEP_END'
    block = 1
  []
  [stress_zz]
    type = RankTwoAux
    rank_two_tensor = cauchy_stress
    variable = stress_zz
    index_i = 2
    index_j = 2
    execute_on = 'TIMESTEP_END'
    block = 1
  []
  [stress_xy]
    type = RankTwoAux
    rank_two_tensor = cauchy_stress
    variable = stress_xy
    index_i = 0
    index_j = 1
    execute_on = 'TIMESTEP_END'
    block = 1
  []
  [stress_xz]
    type = RankTwoAux
    rank_two_tensor = cauchy_stress
    variable = stress_xz
    index_i = 0
    index_j = 2
    execute_on = 'TIMESTEP_END'
    block = 1
  []
  [stress_yz]
    type = RankTwoAux
    rank_two_tensor = cauchy_stress
    variable = stress_yz
    index_i = 1
    index_j = 2
    execute_on = 'TIMESTEP_END'
    block = 1
  []
[]

[Materials]
  [tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1.40625e7
    poissons_ratio = 0.25
    block = 1
  []
  # `stress` is provided here; the Physics action auto-wraps it via
  # ComputeLagrangianWrappedStress (compatibility_mode) and creates
  # the ComputeLagrangianStrain material itself.
  [stress]
    type = ComputeMultiPlasticityStress
    plastic_models = j2
    ep_plastic_tolerance = 1e-9
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
  [top_disp_y]
    type = PiecewiseLinear
    x = '0  1'
    y = '0 -0.1'
  []
[]

[BCs]
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
  [top_deform]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 2
    function = top_disp_y
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  automatic_scaling = true

  petsc_options_iname = '-snes_type -pc_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = 'vinewtonssls lu    NONZERO               1e-12'
  line_search = semismooth

  nl_rel_tol = 1e-9
  nl_abs_tol = 1e-8
  nl_max_its = 10
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
    block = 1
    value_type = max
  []
  [num_nl]
    type = NumNonlinearIterations
  []
  [cumulative_nl]
    type = CumulativeValuePostprocessor
    postprocessor = num_nl
  []
  # Total y-force applied by the contactor at the contact face, obtained
  # by summing the assembled y-residual on the top BC and equilibrium.
  # `save_in` stores R_i = int (gradphi_i : sigma - phi_i*b), which is negative on a
  # top-pushed-down node under compression (sigma_yy < 0), so the raw sum is
  # flipped to give a positive compressive force.
  [contactor_force_raw]
    type = NodalSum
    variable = saved_y
    boundary = 2
    outputs = 'none'
  []
  [contactor_force]
    type = ScalePostprocessor
    value = contactor_force_raw
    scaling_factor = -1.0
  []
  # Contactor is fixed; the "contactor displacement" reported here is
  # the signed applied top displacement (negative going down).  Same
  # convention as the elastic Hertz example.
  [contactor_displacement]
    type = FunctionValuePostprocessor
    function = top_disp_y
  []
[]

[Outputs]
  exodus = true
  csv = true
[]
