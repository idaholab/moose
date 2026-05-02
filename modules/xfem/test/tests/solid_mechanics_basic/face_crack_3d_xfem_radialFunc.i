
[XFEM]
  geometric_cut_userobjects = 'cut_mesh'
  qrule = volfrac
  output_cut_plane = true
[]

[Functions]
  [growth_func_x]
    type = ParsedFunction
    value = '(x-${offset})/sqrt((x-${offset})^2 + z^2)'
  []
  [growth_func_y]
    type = ParsedFunction
    expression = 0.0
  []
  [growth_func_z]
    type = ParsedFunction
    value = 'z/sqrt((x-${offset})^2 + z^2)'
  []
  [growth_func_v]
    type = ParsedFunction
    expression = '.02'
  []
[]

[UserObjects]
  [cut_mesh]
    type = CrackMeshCut3DUserObject
    mesh_generator_name = mesh_cutter
    growth_dir_method = FUNCTION
    growth_direction_x = growth_func_x
    growth_direction_y = growth_func_y
    growth_direction_z = growth_func_z
    size_control = .05
    n_step_growth = 1
    growth_rate = growth_func_v
  []
[]

[Outputs]
  file_base = face_crack_radialFunc_off_${offset}_spin_${spin}
  [xfemcutter]
    type = XFEMCutMeshOutput
    xfem_cutter_uo = cut_mesh
  []
  execute_on=final
[]
