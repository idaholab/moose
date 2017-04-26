[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 19
  ny = 19
[]

[GlobalParams]
  op_num = 12
  var_name_base = gr
  grain_num = 36
[]

[UserObjects]
  [./hex]
    type = PolycrystalHex
    rand_seed = 215
    execute_on = 'initial'
  [../]
[]

[Variables]
  [./PolycrystalVariables]
  [../]
[]

[ICs]
  [./PolycrystalICs]
    [./PolycrystalVoronoiIC]
      polycrystal_uo_ic = hex
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
