# Double cantilever beam peeled apart by per-timestep arc-length continuation,
# with a bilinear cohesive interface ahead of a traction-free pre-crack, in a
# single input: the controllable 'use_continuation' keeps the early steps as
# ordinary prescribed-ramp Newton solves -- the near-zero-strength pre-crack
# strip fails during the first of them at negligible load, and the pre-cracked
# arms then load elastically -- and a [Controls] switch hands the run to the
# continuation just before the crack reaches the first serration of the
# bonded ligament.
#
# The interface strength puts the cohesive zone at the size of one or two
# elements, so the crack advances element by element and every advance is a
# snap-back serration of the load-opening curve: the load and the opening both
# fall while the crack jumps, then the arms reload. The continuation rides the
# whole sawtooth, committing the cohesive damage each step; the two prescribed
# loadings fail on the first tooth:
#   dcb_load_control.i  (Newton under load control: nothing to converge to
#                        above the first peak)
#   dcb_disp_control.i  (Newton under displacement control: cannot follow the
#                        opening reversal of a serration)
#
# Every time step advances the trace by one continuation increment of radius
# step_size and commits the state it reaches, so the cohesive damage
# accumulates increment by increment and the load factor is free to fall where
# the path descends: past the peak the trace sheds load as the crack tears
# through the ligament. Time is a pseudo parameter that counts arc steps; the
# load factor of the trace is the lambda postprocessor. A step that overshoots
# a turn of the path fails and retries with the load span the TimeStepper cut
# back while the radius stays put, and a step that walks back down an elastic
# branch, which a fresh predictor can do at a limit point, descends without
# dissipating and is failed and mirrored by the problem.
#
# Geometry: arms 100 x 3, pre-crack 30, 1 mm elements. lambda = 1 corresponds
# to a peel force of 10 per unit thickness on each arm end.

length = 100
arm = 3
precrack = 30
nx = 100

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [msh]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = ${length}
    ymax = ${fparse 2 * arm}
    nx = ${nx}
    ny = ${fparse 2 * arm}
    elem_type = QUAD4
  []
  [lower_arm]
    type = SubdomainBoundingBoxGenerator
    input = msh
    bottom_left = '0 0 0'
    top_right = '${length} ${arm} 0'
    block_id = 1
    block_name = lower_arm
  []
  [upper_arm]
    type = SubdomainBoundingBoxGenerator
    input = lower_arm
    bottom_left = '0 ${arm} 0'
    top_right = '${length} ${fparse 2 * arm} 0'
    block_id = 2
    block_name = upper_arm
  []
  [split]
    type = BreakMeshByBlockGenerator
    input = upper_arm
  []
  # the free arm ends, split into one sideset per arm so the peel tractions
  # can pull them apart
  [left_lower]
    type = ParsedGenerateSideset
    input = split
    combinatorial_geometry = 'x < 1e-9'
    included_subdomains = lower_arm
    normal = '-1 0 0'
    new_sideset_name = left_lower
  []
  [left_upper]
    type = ParsedGenerateSideset
    input = left_lower
    combinatorial_geometry = 'x < 1e-9'
    included_subdomains = upper_arm
    normal = '-1 0 0'
    new_sideset_name = left_upper
  []
[]

[Physics]
  [SolidMechanics]
    [QuasiStatic]
      [all]
        strain = SMALL
        use_automatic_differentiation = true
        add_variables = true
      []
    []
    [CohesiveZone]
      [interface]
        boundary = 'lower_arm_upper_arm'
      []
    []
  []
[]

[BCs]
  # the continuation load: equal and opposite shear tractions on the free arm
  # ends, an opening couple peeling the arms apart. lambda = 1 is a resultant
  # peel force of 10 per unit thickness on each arm
  [peel_up]
    type = NeumannBC
    variable = disp_y
    boundary = left_upper
    value = ${fparse 10 / arm}
    vector_tags = 'arc_length_load'
  []
  [peel_down]
    type = NeumannBC
    variable = disp_y
    boundary = left_lower
    value = ${fparse -10 / arm}
    vector_tags = 'arc_length_load'
  []
  [clamp_x]
    type = DirichletBC
    variable = disp_x
    boundary = right
    value = 0
  []
  [clamp_y]
    type = DirichletBC
    variable = disp_y
    boundary = right
    value = 0
  []
[]

[Materials]
  [elasticity]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 1.35e5
    poissons_ratio = 0.25
  []
  [stress]
    type = ADComputeLinearElasticStress
  []
  # strengths sized so the cohesive zone spans one or two elements, which is
  # what makes the crack advance element by element and the curve serrated
  # the pre-crack is carved from the cohesive properties: ahead of it the
  # interface carries the real strengths and toughnesses, behind it next to
  # nothing, so the first load increment fails it into free faces
  [czm_properties]
    type = GenericFunctionMaterial
    boundary = 'lower_arm_upper_arm'
    prop_names = 'N_strength S_strength GIc GIIc'
    # the pre-crack strip gets a strength and a toughness so small that its
    # softening window is thinner than any numerical opening: a face there
    # snaps straight to full damage in the first prescribed step, with no
    # partial-softening state to flicker in
    prop_values = 'if(x<${precrack},1e-5,100) if(x<${precrack},1.5e-5,150)
                   if(x<${precrack},1e-12,0.28) if(x<${precrack},3e-12,0.8)'
  []
  [czm]
    type = BiLinearMixedModeTraction
    boundary = 'lower_arm_upper_arm'
    penalty_stiffness = 1e6
    GI_c = GIc
    GII_c = GIIc
    normal_strength = N_strength
    shear_strength = S_strength
    displacements = 'disp_x disp_y'
    eta = 2.2
    # evolve the cohesive damage from the jump of the last committed step: the
    # bilinear law's onset corner is a facet the corrector otherwise
    # limit-cycles across without ever converging an increment, and the
    # per-step commits of the continuation are what bound the lag to one
    # committed increment
    lag_displacement_jump = true
  []
[]

[Problem]
  type = ArcLengthProblem
  # a radius that resolves the serrations element by element, kept through
  # every cutback: a retry shrinks the load span of the step, not the radius
  step_size = 0.005
  psi_squared = 0
  correction_type = normal
[]

[Functions]
  # prescribed-ramp Newton until just before the first serration, continuation
  # after: the first tooth of this loading sits at lambda just below 0.97, so
  # the handoff lands at 0.9 with margin
  [switch_fn]
    type = ParsedFunction
    expression = 't > 0.899'
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

[Postprocessors]
  [lambda]
    type = ArcLengthLoadParameter
  []
  [P]
    type = ScalePostprocessor
    value = lambda
    scaling_factor = 10
    execute_on = 'ARC_LENGTH_INCREMENT TIMESTEP_END'
  []
  [opening]
    type = DifferencePostprocessor
    value1 = tip_up
    value2 = tip_down
    execute_on = 'ARC_LENGTH_INCREMENT TIMESTEP_END'
  []
  [tip_up]
    type = PointValue
    variable = disp_y
    point = '0 ${fparse 2 * arm} 0'
    execute_on = 'ARC_LENGTH_INCREMENT TIMESTEP_END'
  []
  [tip_down]
    type = PointValue
    variable = disp_y
    point = '0 0 0'
    execute_on = 'ARC_LENGTH_INCREMENT TIMESTEP_END'
  []
[]

[VectorPostprocessors]
  [path]
    type = ArcLengthHistory
    postprocessors = 'opening'
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
  solve_type = NEWTON
  line_search = none
  automatic_scaling = true
  dt = 0.01
  # pseudo time: counts arc steps once the continuation owns the run. This
  # span carries the trace over the peak and four serration teeth down the
  # tearing descent, past the loads at which both prescribed loadings fail.
  # Deeper teeth sharpen as the crack lengthens until the corrector cannot
  # converge an increment across one at this radius, so the span ends while
  # every committed step is a converged equilibrium point
  end_time = 4
  dtmin = 1e-12
  nl_max_its = 50
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  csv = true
  exodus = true
[]
