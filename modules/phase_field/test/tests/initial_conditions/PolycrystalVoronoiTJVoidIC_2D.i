[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
  xmax = 10
  ymax = 10
  uniform_refine = 2
[]

[GlobalParams]
  op_num = 8
  var_name_base = gr
  grain_num = 16
  int_width = 0.5
  numbub = 13
  radius = 0.5
  bubspac = 2
  invalue = 1
  outvalue = 0
  numtries = 1e6
  use_kdtree = true
[]

[Variables]
  [./PolycrystalVariables]
  [../]
[]

[AuxVariables]
  [./void]
  [../]
  [./bnds]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[UserObjects]
  [./voronoi_ic_uo]
    type = PolycrystalHex
    coloring_algorithm = bt
  [../]
[]

[ICs]
  [./PolycrystalICs]
    [./PolycrystalVoronoiCoupledVoidIC]
      v = void
      polycrystal_ic_uo = voronoi_ic_uo
    [../]
  [../]
  [./void]
    type = PolycrystalVoronoiTJVoidIC
    variable = void
    polycrystal_ic_uo = voronoi_ic_uo
  [../]
  [./bnds]
    type = BndsCalcIC
    variable = bnds
  [../]
[]

[BCs]
  [./Periodic]
    [./all]
      auto_direction = 'x y'
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
