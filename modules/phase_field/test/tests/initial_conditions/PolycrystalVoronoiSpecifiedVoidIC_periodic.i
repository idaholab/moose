[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 85
  ny = 85
  nz = 0
  xmax = 250
  ymax = 250
  zmax = 0
  elem_type = QUAD4
[]

[GlobalParams]
  op_num = 5
  grain_num = 5
  var_name_base = gr
  radii = '15 15 22 10 20'
  x_positions = '30 120 70 180 200'
  y_positions = '50 60 165 130 250'
  z_positions = '0  0  0   0   0'
  int_width = 10
  invalue = 1
  outvalue = 0.1
[]

[Variables]
  [./c]
  [../]
  [./w]
    scaling = 1.0e4
  [../]
  [./PolycrystalVariables]
  [../]
[]

[BCs]
  [./Periodic]
    [./all]
      auto_direction = 'x y'
    [../]
  [../]
[]

[AuxVariables]
  [./bnds]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[ICs]
  [./PolycrystalICs]
    [./PolycrystalVoronoiSpecifiedVoidIC]
      polycrystal_ic_uo = voronoi
    [../]
  [../]
  [./c_IC]
    variable = c
    type = PolycrystalVoronoiSpecifiedVoidIC
    structure_type = voids
    polycrystal_ic_uo = voronoi
  [../]
[]

[UserObjects]
  [./voronoi]
    type = PolycrystalVoronoi
    rand_seed = 12444
    int_width = 5
  [../]
[]

[Problem]
  type = FEProblem
  solve = false
[]

[AuxKernels]
  [./BndsCalc]
    type = BndsCalcAux
    variable = bnds
    v = 'gr0 gr1 gr2 gr3 gr4 c'
    execute_on = 'timestep_end'
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true
[]
