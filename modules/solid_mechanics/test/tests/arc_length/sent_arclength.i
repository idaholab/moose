# Single-edge-notched tension strip pulled by a uniform dead traction on a
# stiff platen that is free to rotate, with a cohesive ligament across the
# mid-height section: the resultant of the traction stays on the specimen
# axis while the crack growing from the notch shifts the section centroid off
# it, so the moment grows with the crack and the platen tilts. No single
# prescribable displacement controls this structure -- prescribing any one
# displacement leaves the rotation free, and the rotation is what the energy
# release drives -- while clamping the rotation is a different, stiffer
# experiment. Load control dies at the first crack event near 67 kN, below
# the peak near 87 kN that the continuation crosses on its way down the whole
# softening branch. The arc-length trace is the only path through.
#
# The run is a single input: prescribed Newton steps ramp the traction at a
# load factor equal to the time, a [Controls] switch hands the run to the
# continuation below the first crack event, and every later time step
# advances the trace by one committed continuation increment, so the cohesive
# damage accumulates along the path and the load factor is free to fall past
# the peak. The companion input runs the prescribed loading to its failure:
#   sent_load_control.i  (Newton under load control: dies at the first crack
#                         event, before the peak is even reached)
#
# Geometry: 60 mm x 150 mm strip (2D plane strain, unit thickness), 2 mm
# elements, a 6 mm stiff platen bonded on top, and a cohesive interface
# across the whole mid-height section whose first 12 mm carry near-zero
# strength: that segment fails into free faces during the first prescribed
# step and is the notch. lambda = 1 corresponds to a tension of 60 kN per
# meter of thickness.

width = 0.06
height = 0.15
platen = 0.006
notch_depth = 0.012
elem = 0.002
# the ligament must lie on an element line: 0.076 is the 38th row of 2 mm
# elements, one element above mid-height
ligament_y = 0.076

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = ${fparse width / elem}
    ny = ${fparse (height + platen) / elem}
    xmax = ${width}
    ymax = ${fparse height + platen}
    elem_type = QUAD4
  []
  [lower]
    type = SubdomainBoundingBoxGenerator
    input = gen
    block_id = 1
    block_name = lower
    bottom_left = '0 0 0'
    top_right = '${width} ${ligament_y} 0'
  []
  [upper]
    type = SubdomainBoundingBoxGenerator
    input = lower
    block_id = 2
    block_name = upper
    bottom_left = '0 ${ligament_y} 0'
    top_right = '${width} ${height} 0'
  []
  [platen]
    type = SubdomainBoundingBoxGenerator
    input = upper
    block_id = 3
    block_name = platen
    bottom_left = '0 ${height} 0'
    top_right = '${width} ${fparse height + platen} 0'
  []
  # the cohesive ligament: an interface between the two strip halves alone,
  # which leaves the platen bonded to the strip
  [split]
    type = BreakMeshByBlockGenerator
    input = platen
    block_pairs = 'lower upper'
  []
  [bottom_center]
    type = ExtraNodesetGenerator
    input = split
    new_boundary = bottom_center
    coord = '${fparse width / 2} 0'
  []
[]

[Physics]
  [SolidMechanics]
    [QuasiStatic]
      [all]
        strain = SMALL
        use_automatic_differentiation = true
        add_variables = true
        generate_output = 'stress_yy'
      []
    []
    [CohesiveZone]
      [ligament]
        boundary = 'lower_upper'
      []
    []
  []
[]

[Functions]
  # the control evaluates at the end time of the step it arms; the bound sits
  # below the first crack event so the continuation opens from a loaded,
  # still-smooth state
  [switch_fn]
    type = ParsedFunction
    expression = 't > 0.6'
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

[ICs]
  # a nanoscopic initial opening of the interface: the mixed-mode cohesive law
  # has a 0/0 mode mixity at a jump of exactly zero, whose derivatives poison
  # the very first Jacobian of the run, and any nonzero jump clears it. The
  # seed is orders below every physical scale of the problem
  [seed]
    type = FunctionIC
    variable = disp_y
    function = 'if(y > ${ligament_y}, 1e-9, 0)'
  []
[]

[BCs]
  [bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
  [bottom_x]
    type = DirichletBC
    variable = disp_x
    boundary = bottom_center
    value = 0
  []
  # the continuation load: a uniform dead traction on the platen top, whose
  # resultant stays on the specimen axis while the crack shifts the section
  # centroid, so the moment grows with the crack and the platen rotates
  # freely. lambda = 1 is 60 kN per meter of thickness
  [tension]
    type = NeumannBC
    variable = disp_y
    boundary = top
    value = 1e6
    vector_tags = 'arc_length_load'
  []
[]

[Materials]
  [elasticity_strip]
    type = ADComputeIsotropicElasticityTensor
    block = 'lower upper'
    youngs_modulus = 30e9
    poissons_ratio = 0.2
  []
  [elasticity_platen]
    type = ADComputeIsotropicElasticityTensor
    block = platen
    youngs_modulus = 300e9
    poissons_ratio = 0.2
  []
  [stress]
    type = ADComputeLinearElasticStress
  []
  # the notch is carved from the cohesive properties: ahead of it the
  # interface carries the real strength and toughness, behind it next to
  # nothing. The pre-crack strip gets a strength and a toughness so small
  # that its softening window is thinner than any numerical opening: a face
  # there snaps straight to full damage in the first prescribed step, with no
  # partial-softening state to flicker in
  [czm_properties]
    type = GenericFunctionMaterial
    boundary = lower_upper
    prop_names = 'N_strength S_strength GIc GIIc'
    prop_values = 'if(x<${notch_depth},0.3,3e6) if(x<${notch_depth},0.45,4.5e6)
                   if(x<${notch_depth},1e-15,50) if(x<${notch_depth},3e-15,150)'
  []
  [czm]
    type = BiLinearMixedModeTraction
    boundary = lower_upper
    penalty_stiffness = 1e13
    GI_c = GIc
    GII_c = GIIc
    normal_strength = N_strength
    shear_strength = S_strength
    displacements = 'disp_x disp_y'
    eta = 2.2
  []
[]

[Problem]
  type = ArcLengthProblem
  # a radius that resolves the crack-front advances, kept through every
  # cutback: a retry shrinks the load span of the step, not the radius
  step_size = 5e-6
  psi_squared = 0
  correction_type = normal
[]

[Postprocessors]
  [lambda]
    type = ArcLengthLoadParameter
  []
  [P_kN]
    type = ScalePostprocessor
    value = lambda
    scaling_factor = 60
    execute_on = 'ARC_LENGTH_INCREMENT TIMESTEP_END'
  []
  [cmod]
    type = DifferencePostprocessor
    value1 = notch_upper
    value2 = notch_lower
    execute_on = 'ARC_LENGTH_INCREMENT TIMESTEP_END'
  []
  [notch_upper]
    type = PointValue
    variable = disp_y
    point = '0 ${fparse ligament_y + elem} 0'
    execute_on = 'ARC_LENGTH_INCREMENT TIMESTEP_END'
  []
  [notch_lower]
    type = PointValue
    variable = disp_y
    point = '0 ${fparse ligament_y - elem} 0'
    execute_on = 'ARC_LENGTH_INCREMENT TIMESTEP_END'
  []
  # the moment signature: the platen tilt that a prescribed displacement
  # cannot control
  [rotation]
    type = DifferencePostprocessor
    value1 = corner_right
    value2 = corner_left
    execute_on = 'ARC_LENGTH_INCREMENT TIMESTEP_END'
  []
  [corner_left]
    type = PointValue
    variable = disp_y
    point = '0 ${fparse height + platen} 0'
    execute_on = 'ARC_LENGTH_INCREMENT TIMESTEP_END'
  []
  [corner_right]
    type = PointValue
    variable = disp_y
    point = '${width} ${fparse height + platen} 0'
    execute_on = 'ARC_LENGTH_INCREMENT TIMESTEP_END'
  []
[]

[VectorPostprocessors]
  [path]
    type = ArcLengthHistory
    postprocessors = 'cmod rotation'
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
  dt = 0.05
  # pseudo time: counts arc steps once the continuation owns the run
  end_time = 30
  # dt is a pure gauge, so the ladder may go deep at the sharpest crack
  # events, and a ladder that bottoms out ends the trace where it stands
  # rather than erroring: the gold holds every committed row, so a trace that
  # ends early still fails its diff
  dtmin = 1e-12
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
