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
    growth_rate_method = FATIGUE
    growth_direction_x = growth_func_x
    growth_direction_y = growth_func_y
    growth_direction_z = growth_func_z
    fatigue_reporter = "fatigue/growth_increment"
    crack_front_nodes = '7 6 5 4'
  []
[]

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
[]
