[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 40
  ny = 40
  nz = 0
  xmax = 250
  ymax = 250
  zmax = 0
  elem_type = QUAD4
[]

[GlobalParams]
  op_num = 12
  grain_num = 25
  var_name_base = gr
  numbub = 15
  bubspac = 22
  radius = 8
  int_width = 10
  invalue = 1
  outvalue = 0.1
[]

[Variables]
  [c]
  []
  [PolycrystalVariables]
  []
[]

[ICs]
  [PolycrystalICs]
    [PolycrystalVoronoiVoidIC]
      polycrystal_ic_uo = voronoi
    []
  []
  [c_IC]
    variable = c
    type = PolycrystalVoronoiVoidIC
    structure_type = voids
    polycrystal_ic_uo = voronoi
  []
[]

[UserObjects]
  [voronoi]
    type = PolycrystalVoronoi
    int_width = 0
  []
[]

[BCs]
  [Periodic]
    [all]
      auto_direction = 'x y'
    []
  []
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
