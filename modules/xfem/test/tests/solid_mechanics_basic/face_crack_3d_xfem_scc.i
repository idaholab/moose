
[XFEM]
  geometric_cut_userobjects = 'cut_mesh'
  qrule = volfrac
  output_cut_plane = true
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
  block = 0
  incremental = true
[]

[Outputs]
  file_base = face_crack_scc_off_${offset}_spin_${spin}
  [xfemcutter]
    type = XFEMCutMeshOutput
    xfem_cutter_uo = cut_mesh
  []
  execute_on=final
[]
