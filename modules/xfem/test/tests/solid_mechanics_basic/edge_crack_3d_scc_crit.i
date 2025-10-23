# This takes 1 step of max_growth_increment and back computes a time of 11s which
# is the amount of time edge_crack_3d_scc.i takes to grow a crack of similar length.

[Reporters]
  [scc_crack_growth]
    type = StressCorrosionCracking
    growth_increment_name = "crack_growth"
    time_to_max_growth_increment_name = "max_growth_timestep"
    crackMeshCut3DUserObject_name = cut_mesh
    max_growth_increment = 0.1
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
    size_control = 1
    n_step_growth = 1
    growth_increment_method = REPORTER
    growth_reporter = "scc_crack_growth/crack_growth"
    crack_front_nodes = '7 6 5 4'
  []
[]

[DomainIntegral]
  integrals = 'Jintegral InteractionIntegralKI InteractionIntegralKII'
  displacements = 'disp_x disp_y disp_z'
  crack_front_points_provider = cut_mesh
  number_points_from_provider = 4
  crack_direction_method = CurvedCrackFront
  radius_inner = '0.15'
  radius_outer = '0.45'
  poissons_ratio = 0.3
  youngs_modulus = 207000
  block = 0
  incremental = true
[]

#function for BC on top surface
[Functions]
  [top_trac_y]
    type = ConstantFunction
    value = 10
  []
  [top_trac_x]
    type = ConstantFunction
    value = 0
  []
[]

[Outputs]
  file_base = edge_crack_3d_scc_crit_out
  execute_on = 'FINAL'
  [json_out]
    type = JSON
    execute_system_information_on = none
  []
[]
