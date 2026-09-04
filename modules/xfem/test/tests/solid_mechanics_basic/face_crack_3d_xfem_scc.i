
[XFEM]
  geometric_cut_userobjects = 'cut_mesh'
  qrule = volfrac
  output_cut_plane = false
[]

[Reporters]
  [scc_crack_growth]
    type = StressCorrosionCrackingExponential
    growth_increment_name = "crack_growth"
    time_to_max_growth_increment_name = "max_growth_timestep"
    crackMeshCut3DUserObject_name = cut_mesh
    max_growth_increment = 0.02
    k_low = 10
    k_high = 20
    growth_rate_mid_multiplier = 0.00075
    growth_rate_mid_exp_factor = 1
    outputs=none
  []
  # Pass-through copies of the crack front coordinates so they can be output
  # independently of the rest of the scc_crack_growth values.  ParsedVectorReporter
  # declares a single output vector, so x, y, and z each need their own object.
  [crack_front_x]
    type = ParsedVectorReporter
    name = 'x'
    vector_reporter_names = 'scc_crack_growth/x'
    vector_reporter_symbols = 'x'
    expression = 'x'
    execute_on = 'XFEM_MARK TIMESTEP_END'
  []
  [crack_front_y]
    type = ParsedVectorReporter
    name = 'y'
    vector_reporter_names = 'scc_crack_growth/y'
    vector_reporter_symbols = 'y'
    expression = 'y'
    execute_on = 'XFEM_MARK TIMESTEP_END'
  []
  [crack_front_z]
    type = ParsedVectorReporter
    name = 'z'
    vector_reporter_names = 'scc_crack_growth/z'
    vector_reporter_symbols = 'z'
    expression = 'z'
    execute_on = 'XFEM_MARK TIMESTEP_END'
  []
[]

[UserObjects]
  [cut_mesh]
    type = CrackMeshCut3DUserObject
    mesh_generator_name = mesh_cutter
    growth_dir_method = MAX_HOOP_STRESS
    size_control = .05
    n_step_growth = 1
    growth_increment_method = REPORTER
    growth_reporter = "scc_crack_growth/crack_growth"
  []
[]

[DomainIntegral]
  integrals = 'Jintegral InteractionIntegralKI InteractionIntegralKII'
  displacements = 'disp_x disp_y disp_z'
  crack_front_points_provider = cut_mesh
  crack_direction_method = CurvedCrackFront
  radius_inner = '0.025'
  radius_outer = '0.1'
  poissons_ratio = 0.3
  youngs_modulus = 207000
  incremental = false
[]

[Outputs]
  file_base = ${fname}_scc_offset_${offset}_spin_${spin}_tilt_${tilt}
  execute_on = TIMESTEP_END
  [json_out]
    type = JSON
    execute_system_information_on = none
  []
[]
