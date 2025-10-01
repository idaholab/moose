[Reporters]
  [fatigue]
    type = ParisLaw
    growth_increment_name = "growth_increment"
    cycles_to_max_growth_size_name = "fatigue"
    crackMeshCut3DUserObject_name = cut_mesh
    max_growth_size = 0.1
    paris_law_c = 1e-13
    paris_law_m = 2.5
  []
[]

[UserObjects]
  [cut_mesh]
    type = CrackMeshCut3DUserObject
    mesh_generator_name = mesh_cutter
    growth_dir_method = FUNCTION
    size_control = 1
    n_step_growth = 1
    growth_rate_method = REPORTER
    growth_direction_x = growth_func_x
    growth_direction_y = growth_func_y
    growth_direction_z = growth_func_z
    growth_reporter = "fatigue/growth_increment"
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

# function for CrackMeshCut3DUserObject crack
[Functions]
  [growth_func_x]
    type = ParsedFunction
    expression = 1
  []
  [growth_func_y]
    type = ParsedFunction
    expression = 0
  []
  [growth_func_z]
    type = ParsedFunction
    expression = 0
  []
[]

#functino for BC on top surface
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
  file_base = edge_crack_3d_fatigue_out
  json = true
[]
