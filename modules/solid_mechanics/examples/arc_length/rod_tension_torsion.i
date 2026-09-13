# Arc-length continuation demo: a damaging rod under combined tension and torsion
# =============================================================================
#
# WHAT THIS RUN SHOWS
#
# A solid circular rod is clamped at one end. The other end carries a tensile traction and a
# twisting moment that grow together, scaled by one load factor lambda. The middle layer of the rod
# loses stiffness as it strains (scalar damage), so the axial force rises to a peak and then falls
# while the loaded end keeps moving. This run traces the whole force-displacement curve, the
# softening branch after the peak included.
#
# WHY PLAIN LOAD STEPPING CANNOT DO THIS
#
# An ordinary transient ramps the load with the time, lambda = t, and asks Newton for the
# displacement that carries it. Past the peak no displacement carries more load, so the step fails,
# the time step is cut back, and the run dies at the peak. Arc-length continuation makes lambda an
# unknown of the solve and fixes the distance travelled along the equilibrium path per step instead
# of the load carried, so the trace goes over the peak and follows the load down. To watch the plain
# ramp die at the peak, keep the run on prescribed load steps for its whole length:
#
#   ./solid_mechanics-opt -i rod_tension_torsion.i Functions/switch_fn/expression=0
#
# HOW THE FEATURE IS WIRED
#
# 1. [Problem] type = ArcLengthProblem. Under a Transient executioner every time step is one
#    continuation increment whose converged state is committed, which is what a history dependent
#    (damaging) material needs: the damage advances along the path, including down the descent.
# 2. The load objects (the ADPressure pull and the ADTorque twist) carry
#    vector_tags = 'arc_length_load' and matrix_tags = 'arc_length_load_jac'. These REPLACE their
#    default tags, so the loads live only in the load tag that lambda scales. Everything else
#    (stiffness, damage) stays in the default residual.
# 3. The clamped end carries strong DirichletBCs. The loads act on the other end, so no loaded dof
#    is also constrained (a constrained dof would have its load dropped from the residual).
# 4. Both loads are dead loads on the undeformed geometry, so they contribute nothing to the load
#    matrix tag. Carrying the tag costs them nothing and the pairing of the two tags is required.
# 5. The elastic climb needs no continuation. A Controls object keeps use_continuation = false for
#    the first ten steps (plain Newton, lambda = t) and switches it on at t > 0.5, well below the
#    peak. The load factor is continuous through the switch.
#
# OUTPUT AND PLOTTING
#
# rod_tension_torsion_out.csv, one row per time step. Force-displacement curve:
#   x = end_disp      average axial displacement of the loaded end (mm)
#   y = axial_force   axial reaction at the clamped end from the stress (N)
#   y = applied_force lambda times the reference pull on the meshed face, which axial_force
#                     must match (N)
# Load factor: lambda. Torque curve (bonus): x = twist (rad), y = applied_torque (N.mm).
# rod_tension_torsion_out_path.csv holds the same quantities per continuation increment (one per
# step here) without the time column.
#   python3 -c "import pandas as pd, matplotlib.pyplot as plt; d = pd.read_csv('rod_tension_torsion_out.csv'); d.plot(x='end_disp', y=['axial_force', 'applied_force'], marker='.'); plt.show()"
# The Exodus file shows the damage field and the stress on the rod.
#
# EXPECTED NUMBERS (from the closed form solution of the gauge layer in series with the grips)
#
# Peak at lambda ~ 1.46, axial force ~ 890 N (lambda p_ref times the 306 mm^2 face of the 16-gon
# mesh, 97.5 % of the circle), end_disp ~ 0.006 mm. The load falls to about a quarter of the peak
# by end_disp ~ 0.016 mm, which the default end_time reaches in about 70 steps; the tail decays
# like 1 / strain, so extend end_time to go further down. The whole run takes about half a minute
# in serial (240 elements, 882 dofs, direct LU).
#
# UNITS, GEOMETRY AND MATERIAL
#
# mm, MPa, N, N.mm. E = 30 GPa (concrete-like), nu = 0 on purpose: with nu = 0 the clamped end
# causes no lateral constraint, so the strain is uniform within each layer and varies only with the
# radius (the twist strains the outer fibres more). Set nu = 0.3 and a stress concentration at the
# clamp joins the response.
#
# The rod is five layers of 10 mm. Only the middle layer (blocks 'gauge_*') damages; the two layers
# on either side (blocks 'grips_*') stay linear elastic. This is deliberate: a strain-softening LOCAL
# damage model cannot stay uniform past its peak, the damage localizes into one element layer
# wherever round-off picks (tried on this mesh with every layer damaging: the trace reached the
# peak, one layer ran away, and the corrector converged onto a fully damaged rod at zero load).
# Making the gauge layer one element thick fixes where and how wide the softening band is, which
# is what a nonlocal or gradient damage model would otherwise do. The grips unload elastically
# while the gauge layer softens, so the end displacement keeps growing on the descent as long as
# the softening slope of the law stays below E h / (L - h) = E / 4; the law below is bounded at
# E / 8.
#
# Damage law (in [Materials]): d = s / (1 + s) with s = (eps_1 / eps0)^2, eps_1 the largest
# principal strain, made irreversible by
# holding the committed value d_old whenever the driver falls below it. In pure tension this is
# sigma = E eps / (1 + (eps / eps0)^2): smooth, peak at eps = eps0 with sigma_peak = E eps0 / 2,
# steepest softening slope -E / 8, tail ~ 1 / eps. The damage is applied through
# ADScalarMaterialDamage + ADComputeDamageStress with the current (not lagged) damage, so the AD
# tangent carries the softening and the corrector can round the limit point within a step. The
# smoothness matters: a corner in the law makes the Newton corrector limit cycle across it, and
# then use_old_damage = true on the damage model would be the remedy.
#
# TUNING KNOBS (all at the top of the file or marked 'knob' below)
#
# eps0         strain scale of the damage law; sets the peak strain and the length of the tail
# p_ref        reference traction at lambda = 1; the peak sits at lambda ~ E eps0 / (2 p_ref)
# shear_ratio  outer-fibre shear strain / axial strain on the elastic branch (torque size); 0.5
#              keeps the shear a visible but secondary driver of the damage
# step_size    arc-length radius per step, in units of the L2 norm of the whole displacement dof
#              vector (mm). Size it on the sharpest feature of the path, not on the smooth climb
# dt           load span per step: lambda moves by at most dt per step in the direction of travel,
#              so on the climb lambda = t, and past the peak it falls by up to dt per step
# end_time     how far down the tail the trace runs (the pseudo time counts steps, not load)
# switch_fn    where the plain ramp hands over to the continuation; keep it below the peak

radius = 10
length = 50
layers = 5
eps0 = 2e-4
p_ref = 2
shear_ratio = 0.5

# Reference torque that gives an outer-fibre shear strain of shear_ratio times the axial strain on
# the elastic branch: tau_max = G gamma = (E / 2)(shear_ratio p_ref / E), T = tau_max J / R with
# J = pi R^4 / 2, so T = shear_ratio p_ref pi R^3 / 4.
T_ref = ${fparse shear_ratio * p_ref * pi * radius^3 / 4}
layer = ${fparse length / layers}

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  # Solid disc (rmin = 0 gives TRI3 at the centre and QUAD4 rings), extruded along z into PRISM6
  # and HEX8 layers, 16 x 3 x 5 = 240 elements. The solution is uniform within each layer, so the
  # axial resolution only has to keep the element aspect ratio reasonable. Refining costs wall time
  # only: the AD Jacobian assembly and the LU solve dominate each step.
  [disc]
    type = AnnularMeshGenerator
    nt = 16
    nr = 3
    rmin = 0
    rmax = ${radius}
  []
  [rod]
    type = AdvancedExtruderGenerator
    input = disc
    direction = '0 0 1'
    heights = '${length}'
    num_layers = '${layers}'
    bottom_boundary = bottom
    top_boundary = top
  []
  # The middle layer is the damaging gauge layer; everything else is the elastic grips. The HEX8
  # rings (block 0) and the PRISM6 core (block 1) stay in separate blocks because Exodus output
  # needs one element type per block, so each region is a pair of blocks.
  [gauge_hex]
    type = SubdomainBoundingBoxGenerator
    input = rod
    restricted_subdomains = 0
    block_id = 2
    bottom_left = '${fparse -radius} ${fparse -radius} ${fparse 2 * layer}'
    top_right = '${radius} ${radius} ${fparse 3 * layer}'
  []
  [gauge_prism]
    type = SubdomainBoundingBoxGenerator
    input = gauge_hex
    restricted_subdomains = 1
    block_id = 3
    bottom_left = '${fparse -radius} ${fparse -radius} ${fparse 2 * layer}'
    top_right = '${radius} ${radius} ${fparse 3 * layer}'
  []
  [names]
    type = RenameBlockGenerator
    input = gauge_prism
    old_block = '0 1 2 3'
    new_block = 'grips_hex grips_prism gauge_hex gauge_prism'
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    strain = SMALL
    # ADComputeDamageStress builds on the incremental elastic stress update and needs the strain
    # increment; for small rotations the incremental small strain equals the total small strain.
    incremental = true
    use_automatic_differentiation = true
    add_variables = true
    generate_output = 'stress_zz strain_zz'
  []
[]

[Functions]
  [switch_fn]
    # knob: plain Newton ramp for t <= 0.5 (lambda = t), continuation after. Set to 0 for the
    # load-control run that dies at the peak, or to 1 to continue from the first step.
    type = ParsedFunction
    expression = 't > 0.5'
  []
[]

[Controls]
  [switch]
    type = BoolFunctionControl
    parameter = '*/*/use_continuation'
    function = switch_fn
    execute_on = 'initial timestep_begin'
  []
[]

[BCs]
  # Clamped end: all three components. The loads never touch this face.
  [fix_x]
    type = DirichletBC
    variable = disp_x
    boundary = bottom
    value = 0
  []
  [fix_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
  [fix_z]
    type = DirichletBC
    variable = disp_z
    boundary = bottom
    value = 0
  []

  # Axial pull: a Pressure with a negative factor is a tensile traction along the outward normal.
  # The top face normal is +z, so only the disp_z component receives a contribution.
  # use_displaced_mesh = false makes it a dead load consistent with the small strain kinematics.
  [pull]
    type = ADPressure
    variable = disp_z
    boundary = top
    factor = ${fparse -p_ref}
    use_displaced_mesh = false
    vector_tags = 'arc_length_load'
    matrix_tags = 'arc_length_load_jac'
  []

  # Twist: tractions distributed over the top face whose resultant moment about the axis is
  # 'direction' (the torque vector). The magnitude is normalized by the polar moment of inertia
  # postprocessor. The traction has no z component, so only disp_x and disp_y are loaded.
  [twist_x]
    type = ADTorque
    variable = disp_x
    boundary = top
    origin = '0 0 ${length}'
    direction = '0 0 ${T_ref}'
    polar_moment_of_inertia = pmi
    vector_tags = 'arc_length_load'
    matrix_tags = 'arc_length_load_jac'
  []
  [twist_y]
    type = ADTorque
    variable = disp_y
    boundary = top
    origin = '0 0 ${length}'
    direction = '0 0 ${T_ref}'
    polar_moment_of_inertia = pmi
    vector_tags = 'arc_length_load'
    matrix_tags = 'arc_length_load_jac'
  []
[]

[Materials]
  [elasticity]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 30000
    poissons_ratio = 0
  []
  # Grips: linear elastic, written in the incremental form the Physics block produces.
  [stress_grips]
    type = ADComputeFiniteStrainElasticStress
    block = 'grips_hex grips_prism'
  []

  # Strain measure that drives the damage in the gauge layer: the largest principal strain, from
  # an eigen decomposition that keeps the AD derivatives. Under the twist it is
  # eps/2 + sqrt(eps^2/4 + gamma^2/4) at the outer fibres, so the torsion drives the damage too.
  # This is the one tensor-to-scalar material in the module that keeps the derivatives:
  # ADRankTwoCartesianComponent, ADRankTwoInvariant and ADStrainEnergyDensity all strip them,
  # which leaves the softening out of the tangent and turns Newton into a slow secant iteration
  # (measured: 6 to 28 iterations per step on the climb, and past the peak the predictor walks
  # back down the elastic branch at every step until the run dies at dtmin).
  [principal_strain]
    type = ADEigenDecompositionMaterial
    rank_two_tensor = mechanical_strain
    base_name = strain
    block = 'gauge_hex gauge_prism'
  []

  # Rational softening law evaluated at the current strain (reversible on its own).
  [damage_new]
    type = ADParsedMaterial
    property_name = damage_new
    material_property_names = 'strain_max_eigen_value'
    constant_names = 'eps0'
    constant_expressions = '${eps0}'
    expression = '1 - 1 / (1 + (strain_max_eigen_value / eps0)^2)'
    block = 'gauge_hex gauge_prism'
  []
  # Damage driver: the law above made irreversible by holding its own committed value from the
  # previous step (Old[...]) whenever the strain falls, which is what makes the material path
  # dependent and the per-step commits of the transient continuation necessary. The switch
  # matters: every committed state sits exactly on it, and the branch taken there is the tangent
  # the next step's predictor starts from. The 1e-9 tolerance keeps the loading branch, whose AD
  # derivative carries the softening, selected at the committed state (it lets the damage fall by
  # 1e-9 on unloading, which is nothing). Without it the predictor starts on the elastic branch
  # and can walk back down it, which converges just as well as a descent along the softening
  # branch and commits a state off the path.
  [damage_driver]
    type = ADParsedMaterial
    property_name = damage_driver
    material_property_names = 'damage_new d_old:=Old[damage_driver]'
    expression = 'if(damage_new >= d_old - 1e-9, damage_new, d_old)'
    block = 'gauge_hex gauge_prism'
  []
  # The damage model reads the driver and scales the elastic stress by (1 - d). use_old_damage is
  # left false so the stress carries the current damage and the AD tangent sees the softening.
  [damage]
    type = ADScalarMaterialDamage
    damage_index = damage_driver
    block = 'gauge_hex gauge_prism'
  []
  [stress_gauge]
    type = ADComputeDamageStress
    damage_model = damage
    block = 'gauge_hex gauge_prism'
  []
[]

[AuxVariables]
  [damage]
    order = CONSTANT
    family = MONOMIAL
  []
  [rotation]
  []
[]

[AuxKernels]
  # Both execute on ARC_LENGTH_INCREMENT as well, so the values sampled by the path history are
  # those of the increment just converged rather than of the one before.
  [damage]
    type = ADMaterialRealAux
    variable = damage
    property = damage_index
    block = 'gauge_hex gauge_prism'
    execute_on = 'ARC_LENGTH_INCREMENT TIMESTEP_END'
  []
  [rotation]
    # Rotation angle of each node about the rod axis, from its displacement; zero on the axis.
    type = RotationAngle
    variable = rotation
    origin = '0 0 0'
    direction = '0 0 1'
    execute_on = 'ARC_LENGTH_INCREMENT TIMESTEP_END'
  []
[]

[Problem]
  type = ArcLengthProblem
  # knob: radius of one increment in the norm of the displacement dof vector (mm). A step of
  # dt = 0.05 in the load factor moves that norm by about 0.002 on the climb and by about the same
  # on this descent, so at 0.004 the load span sets the resolution of the whole trace and the
  # radius only bounds the displacement excursion of a step that would exceed it, which is what a
  # sharper turn of the path needs. Halve it to watch the radius take over on the descent. The norm
  # grows with the mesh (more dofs) and with the rod length, so resize this after changing either.
  step_size = 0.004
  # Cylindrical constraint: the arc length is measured with the displacements alone.
  psi_squared = 0
  # Correct onto the hyperplane normal to the increment; always has a real solution, unlike the
  # exact quadratic correction where the path turns sharply relative to step_size.
  correction_type = normal
  # lambda_min, lambda_max and max_continuation_steps belong to a one-shot Steady trace and are
  # errors under Transient: end_time ends this run.
[]

[Postprocessors]
  [pmi]
    # Polar moment of inertia of the loaded face about the rod axis; normalizes the torque
    # tractions. The face is fixed in the undeformed configuration, so once at INITIAL is enough.
    type = PolarMomentOfInertia
    boundary = top
    origin = '0 0 ${length}'
    direction = '0 0 1'
    execute_on = 'INITIAL'
  []
  [area]
    # Area of the meshed loaded face (a 16-gon, 306 mm^2, not pi R^2), fixed, so once at INITIAL.
    type = AreaPostprocessor
    boundary = top
    execute_on = 'INITIAL'
  []
  [lambda]
    type = ArcLengthLoadParameter
  []
  [axial_force]
    # Integral of the traction sigma.n over the clamped face. Its outward normal is -z, so the
    # direction is flipped to report a tensile force as positive.
    type = ADSidesetReaction
    boundary = bottom
    stress_tensor = stress
    direction = '0 0 -1'
    use_displaced_mesh = false
    execute_on = 'ARC_LENGTH_INCREMENT TIMESTEP_END'
  []
  [applied_force]
    # lambda times the reference pull over the meshed face; equals axial_force at every converged
    # state, which is the equilibrium check of the trace.
    type = ParsedPostprocessor
    expression = 'lambda * ${p_ref} * area'
    pp_names = 'lambda area'
    execute_on = 'ARC_LENGTH_INCREMENT TIMESTEP_END'
  []
  [applied_torque]
    type = ScalePostprocessor
    value = lambda
    scaling_factor = ${T_ref}
    execute_on = 'ARC_LENGTH_INCREMENT TIMESTEP_END'
  []
  [end_disp]
    type = SideAverageValue
    variable = disp_z
    boundary = top
    execute_on = 'ARC_LENGTH_INCREMENT TIMESTEP_END'
  []
  [twist]
    # Average rotation of the loaded face about the axis (rad).
    type = SideAverageValue
    variable = rotation
    boundary = top
    execute_on = 'ARC_LENGTH_INCREMENT TIMESTEP_END'
  []
  [max_damage]
    type = ElementExtremeValue
    variable = damage
    execute_on = 'ARC_LENGTH_INCREMENT TIMESTEP_END'
  []
[]

[VectorPostprocessors]
  [path]
    # lambda plus these postprocessors at every continuation increment, written as one file with
    # the complete history: rod_tension_torsion_out_path.csv.
    type = ArcLengthHistory
    postprocessors = 'end_disp axial_force twist max_damage'
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
  # NEWTON is required: the continuation solves with the assembled Jacobian. Do not set -snes_type,
  # the problem owns it.
  solve_type = NEWTON
  line_search = none
  automatic_scaling = true
  # knob: load span per step. On the climb lambda advances by dt per step (lambda = t); past the
  # peak lambda falls by up to dt per step and parts from the time, which keeps counting steps.
  dt = 0.05
  # knob: about 70 steps, which carries the trace down to about a quarter of the peak load.
  end_time = 3.5
  # A step that fails is retried on a smaller load span with the same radius. If the cutbacks
  # reach dtmin the run stops there and keeps the history written so far (the load-control
  # variant ends this way at the peak, after 13 failed attempts).
  dtmin = 1e-4
  error_on_dtmin = false
  nl_max_its = 40
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  csv = true
  exodus = true
[]
