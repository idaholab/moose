[Mesh]
  file = prepare_mesh_out.e
[]

[Variables]
  [./PolycrystalVariables]
    op_num = 4
    var_name_base = gr
    initial_from_file = true
  [../]
[]

[Executioner]
  type = Steady
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Outputs]
  exodus = true
  execute_on = FINAL
[]
