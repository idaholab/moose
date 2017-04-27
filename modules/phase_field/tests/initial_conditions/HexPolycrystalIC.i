[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 19
  ny = 19
[]

[GlobalParams]
  op_num = 9
  var_name_base = gr
  grain_num = 36
[]

[Variables]
  [./PolycrystalVariables]
  [../]
[]

[UserObjects]
  [./hex_ic]
    type = PolycrystalHex
    coloring_algorithm = bt
  [../]
[]

[ICs]
  [./PolycrystalICs]
    [./PolycrystalColoringIC]
      polycrystal_ic_uo = hex_ic
    [../]
  [../]
[]

[BCs]
  [./Periodic]
    [./all]
      auto_direction = 'x'
    [../]
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Outputs]
  [./out]
    type = Exodus
    execute_on = final
  [../]
[]
