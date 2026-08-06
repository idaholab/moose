[XFEM]
  geometric_cut_userobjects = cut_mesh
  qrule = volfrac
  output_cut_plane = false
[]

[Reporters]
  [critical_crack_growth]
    type = CriticalCrackGrowth
    growth_increment_name = crack_growth
    crackMeshCut3DUserObject_name = cut_mesh
    max_growth_increment = 0.02
    k_critical = 10
  []
[]

[UserObjects]
  [cut_mesh]
    type = CrackMeshCut3DUserObject
    mesh_generator_name = mesh_cutter
    growth_dir_method = MAX_HOOP_STRESS
    size_control = 0.05
    n_step_growth = 1
    growth_increment_method = REPORTER
    growth_reporter = critical_crack_growth/crack_growth
  []
[]

[DomainIntegral]
  integrals = 'Jintegral InteractionIntegralKI InteractionIntegralKII'
  displacements = 'disp_x disp_y disp_z'
  crack_front_points_provider = cut_mesh
  crack_direction_method = CurvedCrackFront
  radius_inner = 0.025
  radius_outer = 0.1
  poissons_ratio = 0.3
  youngs_modulus = 207000
  incremental = false
[]

[Outputs]
  file_base = face_crack_critical_growth_out
  execute_on = TIMESTEP_END
  [json_out]
    type = JSON
    execute_system_information_on = none
  []
[]
