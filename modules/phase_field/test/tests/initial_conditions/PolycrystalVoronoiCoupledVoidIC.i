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
  invalue = 1
  outvalue = 0.1
  int_width = 10
[]

[Variables]
  [./c]
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
    [./PolycrystalVoronoiCoupledVoidIC]
      polycrystal_ic_uo = voronoi
      v = c # coupled voids
    [../]
  [../]
  [./c_IC] # Create voids
    type = MultiSmoothCircleIC
    variable = c
    numbub = 10
    bubspac = 60
    radius = 15
  [../]
[]

[UserObjects]
  [./voronoi]
    type = PolycrystalVoronoi
    rand_seed = 12444
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
