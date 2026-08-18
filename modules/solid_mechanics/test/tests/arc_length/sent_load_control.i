# The strip of sent_arclength.i under plain load control: the same dead
# traction is ramped directly with time, load factor = t, and every step is
# an ordinary Newton solve. The ramp dies at the first crack event of the
# ligament, near 67 kN and below the 87 kN peak: Newton cannot converge
# across the event, the TimeStepper cuts back to dtmin, and the run aborts.
# The arc-length run crosses the event, finds the peak, and traces the whole
# softening branch where this one stops.

width = 0.06
height = 0.15
platen = 0.006
notch_depth = 0.012
elem = 0.002
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
  # the same load, ramped directly: load factor = t
  [tension]
    type = FunctionNeumannBC
    variable = disp_y
    boundary = top
    function = '1e6 * t'
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
    # the live law: lagging the jump under a prescribed ramp converges every
    # step on the damage of the step before and rides through the true limit
    # on fictitious stiffness, while plain Newton on the live law has nothing
    # to converge to above the peak, which is the failure this input records
    lag_displacement_jump = false
  []
[]

[Postprocessors]
  [P_kN]
    type = FunctionValuePostprocessor
    function = '60 * t'
  []
  [cmod]
    type = DifferencePostprocessor
    value1 = notch_upper
    value2 = notch_lower
    execute_on = TIMESTEP_END
  []
  [notch_upper]
    type = PointValue
    variable = disp_y
    point = '0 ${fparse ligament_y + elem} 0'
    execute_on = TIMESTEP_END
  []
  [notch_lower]
    type = PointValue
    variable = disp_y
    point = '0 ${fparse ligament_y - elem} 0'
    execute_on = TIMESTEP_END
  []
  # the moment signature: the platen tilt that a prescribed displacement
  # cannot control
  [rotation]
    type = DifferencePostprocessor
    value1 = corner_right
    value2 = corner_left
    execute_on = TIMESTEP_END
  []
  [corner_left]
    type = PointValue
    variable = disp_y
    point = '0 ${fparse height + platen} 0'
    execute_on = TIMESTEP_END
  []
  [corner_right]
    type = PointValue
    variable = disp_y
    point = '${width} ${fparse height + platen} 0'
    execute_on = TIMESTEP_END
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
  end_time = 3
  dtmin = 1e-6
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
