[UserObjects]
  [cut_mesh]
    type = CrackMeshCut3DUserObject
    mesh_file = mesh_edge_crack.xda
    growth_dir_method = FUNCTION
    size_control = 0.1
    n_step_growth = 1
    growth_direction_x = growth_func_x
    growth_direction_y = growth_func_y
    growth_direction_z = growth_func_z
    growth_rate = growth_func_v
  []
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
  [growth_func_v]
    type = ParsedFunction
    expression = 0.15
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
  file_base = edge_crack_3d_propagation_out
[]
