# Example: plate rolling with two-pass thickness reduction against
# rigid rollers, 3D quarter-symmetry, large-deformation J2 plasticity.
#
# Physical setup (full plate before symmetry reduction):
#   plate      height x width x length  =  1 x 2 x 4
#   roll 1     radius 2, axis // x,     centered at (any, 2.375, +1)
#              bottom edge at y = +0.375  ->  reduces full-plate height 1 -> 0.75
#   roll 2     radius 2, axis // x,     centered at (any, 2.25,  +3)
#              bottom edge at y = +0.25   ->  reduces full-plate height 0.75 -> 0.5
#
# The back face at z = -4 is displacement-driven in the +z direction; the
# plate is fed forward through the two rollers.  Quarter symmetry uses
# the x = 0 mid-width plane and y = 0 mid-height plane, so the modeled
# domain occupies x in [0, 1], y in [0, 0.5], z in [-4, 0] initially.
# Both cylinders' axes are along +x and therefore intersect the x = 0
# symmetry plane; only their half (x >= 0) enters the level set, so the
# quarter model reproduces the full-plate rolling force exactly (times 1/4
# from the two symmetry planes).
#
# Material: rate-independent J2 plasticity with a deliberately low yield
# stress and mild power-law hardening so the plate flows plastically as
# soon as it enters either roller gap; without the low yield the plate
# would deform elastically and pop back out between rollers.
#
# Solver: `newtonls` + LU + `basic` line search, no bounds -- the same
# setup used in the force-controlled plastic example.  Contact is
# displacement-controlled here (the back face drives the flow, the
# rollers just clamp the plate top), so no load-control layer is needed.

# Mesh-resolution knob.  nx / ny / nz here go into GeneratedMeshGenerator
# directly; bump them for a converged rolling force and a smoother roller
# footprint.  Defaults keep the run under a minute on 8 ranks.
nx = 6
ny = 6
nz = 40

# End-of-run push distance.  Chosen so the initial front face at z = 0 has
# been fed past both rollers (z_1 = 1, z_2 = 3) with a margin.
push_distance = 4.0
end_time      = 1.0

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [plate]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 0.5
    zmin = -4
    zmax = 0
    nx = ${nx}
    ny = ${ny}
    nz = ${nz}
    # Standard face labels: left/right (x), bottom/top (y), back/front (z).
    boundary_name_prefix = 'plate'
  []
  # Shared lower-d block on the plate's top face -- both rollers hang their
  # LM variables and NCP kernels off of this one block.  Two [RigidContact]
  # sub-blocks that each emit their own LowerDBlockFromSidesetGenerator on
  # the same sideset would collide (only the first would actually attach),
  # so we build one block up front and tell both sub-blocks to reuse it.
  [contact_lower]
    type = LowerDBlockFromSidesetGenerator
    input = plate
    sidesets = 'plate_top'
    new_block_id   = 10001
    new_block_name = contact_lower
  []
  allow_renumbering = false
[]

[Variables]
  [disp_x]
    block = '0 contact_lower'
  []
  [disp_y]
    block = '0 contact_lower'
  []
  [disp_z]
    block = '0 contact_lower'
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    strain = FINITE
    add_variables = false
    compatibility_mode = true
    decomposition_method = EigenSolution
    # VLC left off here: with the deliberately low yield stress below and
    # the large volumetric plastic flow under the rollers, VLC's stabilized
    # strain and the eigen-decomposition kinematics interact to produce a
    # nonsymmetric intermediate rate tensor and abort with
    # "The tensor is not symmetric" in FactorizedRankTwoTensor.  The plate
    # mesh is coarse enough (hex8, no near-incompressibility) that skipping
    # VLC is fine for this demonstration.
    volumetric_locking_correction = false
    block = 0
  []
[]

[UserObjects]
  [roll1]
    type = InfiniteCylinderContactor
    origin = '0 2.375 1'
    axis   = '1 0 0'
    radius = 2.0
  []
  [roll2]
    type = InfiniteCylinderContactor
    origin = '0 2.25  3'
    axis   = '1 0 0'
    radius = 2.0
  []
  [yield_strength]
    type = SolidMechanicsHardeningPowerRule
    # Low relative to E = 200 (yield stress here is ~1/40 of Young's modulus)
    # so that as soon as the plate top touches a roller it enters the plastic
    # branch instead of springing back elastically.  Modest power-law
    # hardening prevents runaway localization at the roller-gap entry.
    value_0  = 5.0
    epsilon0 = 0.02
    exponent = 0.4
  []
  [j2]
    type = SolidMechanicsPlasticJ2
    yield_strength = yield_strength
    yield_function_tolerance      = 1e-4
    internal_constraint_tolerance = 1e-9
  []
[]

[RigidContact]
  [roll1]
    contactor = roll1
    boundary  = plate_top
    displacements = 'disp_x disp_y disp_z'
    lm_variable_name   = normal_lm_roll1
    lower_d_block_name = contact_lower
    # Shared lower-d block emitted in [Mesh] above; both sub-blocks reuse it.
    add_lower_d_block = false
    enforce_bounds    = false
  []
  [roll2]
    contactor = roll2
    boundary  = plate_top
    displacements = 'disp_x disp_y disp_z'
    lm_variable_name   = normal_lm_roll2
    lower_d_block_name = contact_lower
    add_lower_d_block = false
    enforce_bounds    = false
  []
[]

[AuxVariables]
  [plastic_strain_mag]
    order = CONSTANT
    family = MONOMIAL
    block = 0
  []
  [saved_x]
    block = 0
  []
  [saved_y]
    block = 0
  []
  [saved_z]
    block = 0
  []
[]

[AuxKernels]
  [plastic_strain_mag]
    type = MaterialRealAux
    property = eff_plastic_strain
    variable = plastic_strain_mag
    execute_on = 'TIMESTEP_END'
    block = 0
  []
[]

[Materials]
  [tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 200
    poissons_ratio = 0.3
    block = 0
  []
  # `stress` published here; the Physics action auto-wraps it via
  # ComputeLagrangianWrappedStress (compatibility_mode).
  [stress]
    type = ComputeMultiPlasticityStress
    plastic_models = j2
    ep_plastic_tolerance = 1e-9
    block = 0
  []
  [eff_plastic_strain]
    type = RankTwoInvariant
    rank_two_tensor = plastic_strain
    property_name = eff_plastic_strain
    invariant = EffectiveStrain
    block = 0
  []
[]

[Functions]
  # Linear ramp -- disp_z on the back face increases from 0 to push_distance
  # over t in [0, end_time], feeding the plate through the rollers.
  [back_push]
    type = PiecewiseLinear
    x = '0 ${end_time}'
    y = '0 ${push_distance}'
  []
[]

[BCs]
  [symm_x]
    type = DirichletBC
    variable = disp_x
    boundary = plate_left    # x = 0 symmetry plane
    value = 0.0
  []
  [symm_y]
    type = DirichletBC
    variable = disp_y
    boundary = plate_bottom  # y = 0 symmetry plane
    value = 0.0
  []
  [push_back]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = plate_back    # z = -4 back face
    function = back_push
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  automatic_scaling = true

  petsc_options_iname = '-snes_type -pc_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = 'newtonls    lu       NONZERO               1e-12'
  line_search = basic

  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-6
  nl_max_its = 40
  l_max_its  = 200

  start_time = 0.0
  end_time   = ${end_time}

  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 0.005
    growth_factor  = 1.5
    cutback_factor = 0.5
    optimal_iterations = 8
    iteration_window   = 2
  []
[]

[Postprocessors]
  [max_lm_roll1]
    type = NodalExtremeValue
    variable = normal_lm_roll1
    block = contact_lower
    value_type = max
  []
  [max_lm_roll2]
    type = NodalExtremeValue
    variable = normal_lm_roll2
    block = contact_lower
    value_type = max
  []
  [max_plastic_strain]
    type = ElementExtremeValue
    variable = plastic_strain_mag
    block = 0
    value_type = max
  []
  [num_nl]
    type = NumNonlinearIterations
  []
  [cumulative_nl]
    type = CumulativeValuePostprocessor
    postprocessor = num_nl
  []
  # z-displacement applied at the back face (piecewise-linear ramp).
  [contactor_displacement]
    type = FunctionValuePostprocessor
    function = back_push
  []
[]

[Outputs]
  exodus = true
  csv = true
[]
