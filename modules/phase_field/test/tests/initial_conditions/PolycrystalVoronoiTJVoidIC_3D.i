
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 5
  ny = 5
  nz = 5
  xmax = 10
  ymax = 10
  zmax = 10
  uniform_refine = 2
[]

[GlobalParams]
  op_num = 5
  grain_num = 5
  var_name_base = etam
  int_width = 0.5
  numbub = 10
  radius = 1
  bubspac = 2.5
  invalue = 1.0
  outvalue = 0.0
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
      polycrystal_ic_uo = voronoi_ic_uo
      v = void
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
    type = PolycrystalVoronoi
    coloring_algorithm = jp
  [../]
[]

[BCs]
  [./Periodic]
    [./All]
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
