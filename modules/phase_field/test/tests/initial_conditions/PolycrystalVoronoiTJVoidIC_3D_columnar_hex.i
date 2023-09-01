
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 5
  ny = 5
  nz = 5
  xmax = 10
  ymax = 10
  zmax = 5
  uniform_refine = 2
[]

[GlobalParams]
  op_num = 8
  grain_num = 16
  var_name_base = etam
  int_width = 0.25
  numbub = 108
  radius = 0.5
  bubspac = 1.0
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
  [./bnds]
    order = FIRST
    family = LAGRANGE
  [../]
  [./void]
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

[UserObjects]
  [./voronoi_ic_uo]
    type = PolycrystalHex
    coloring_algorithm = jp
    columnar_3D = true
  [../]
[]

[BCs]
  [./Periodic]
    [./All] # AEH disp
      auto_direction = 'x y z'
    [../]
  [../]
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  [./out]
    type = Exodus
    execute_on = final
  [../]
[]
