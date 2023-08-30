[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 20
  ny = 20
  nz = 1
  xmax = 1
  ymax = 1
  zmax = 0.1
[]

[GlobalParams]
  op_num = 4
  grain_num = 4
  var_name_base = gr
  int_width = 0.05
[]

[Variables]
  [./PolycrystalVariables]
  [../]
[]

[ICs]
  [./PolycrystalICs]
    [./PolycrystalColoringIC]
      polycrystal_ic_uo = hex_ic
    [../]
  [../]
  [./bnds]
    type = BndsCalcIC
    variable = bnds
  [../]
[]

[AuxVariables]
  [./bnds]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[UserObjects]
  [./hex_ic]
    type = PolycrystalHex
    coloring_algorithm = bt
    columnar_3D = true
  [../]
[]

[BCs]
  [./Periodic]
    [./all]
      auto_direction = 'x y'
    [../]
  [../]
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 0
[]

[Outputs]
  exodus = true
[]
