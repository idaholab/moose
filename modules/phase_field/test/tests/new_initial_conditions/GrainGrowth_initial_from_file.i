[Mesh]
  file = prepare_mesh_out.e
[]

[Modules]
  [./PhaseField]
    [./GrainGrowth]
      op_num = 4
      var_name_base = gr
      initial_from_file = true
      variable_mobility = false
    [../]
  [../]
[]

[Materials]
  [./const]
    type = GenericConstantMaterial
    prop_names = 'L mu gamma_asymm kappa_op'
    prop_values = '0 0 0 0'
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 2
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Outputs]
  exodus = true
  execute_on = FINAL
  hide = bnds
[]
