[Mesh]
  [drmg]
    type = DistributedRectilinearMeshGenerator
    dim = 2
    nx = 30
    ny = 30
    nz = 0
    xmin = 0
    xmax = 1000
    ymin = 0
    ymax = 1000
    zmin = 0
    zmax = 0
    elem_type = QUAD4
    partition = linear
  []
[]

[GlobalParams]
  op_num = 4
  var_name_base = gr
[]

[Variables]
  [./PolycrystalVariables]
  [../]
[]

[UserObjects]
  [./voronoi]
    type = PolycrystalVoronoi
    rand_seed = 105
    grain_num = 4
    coloring_algorithm = bt
  [../]
[]

[ICs]
  [./PolycrystalICs]
    [./PolycrystalColoringIC]
      polycrystal_ic_uo = voronoi
    [../]
  [../]
[]

[AuxVariables]
  [./bnds]
    order = FIRST
    family = LAGRANGE
  [../]
  [ghosting0]
    order = CONSTANT
    family = MONOMIAL
  []
  [ghosting1]
    order = CONSTANT
    family = MONOMIAL
  []
  [ghosting2]
    order = CONSTANT
    family = MONOMIAL
  []
  [evaluable0]
    order = CONSTANT
    family = MONOMIAL
  []
  [evaluable1]
    order = CONSTANT
    family = MONOMIAL
  []
  [evaluable2]
    order = CONSTANT
    family = MONOMIAL
  []
  [proc]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [ghosting0]
    type = ElementUOAux
    variable = ghosting0
    element_user_object = ghosting_uo0
    field_name = "ghosted"
    execute_on = initial
  []
  [ghosting1]
    type = ElementUOAux
    variable = ghosting1
    element_user_object = ghosting_uo1
    field_name = "ghosted"
    execute_on = initial
  []
  [ghosting2]
    type = ElementUOAux
    variable = ghosting2
    element_user_object = ghosting_uo2
    field_name = "ghosted"
    execute_on = initial
  []
  [evaluable0]
    type = ElementUOAux
    variable = evaluable0
    element_user_object = ghosting_uo0
    field_name = "evaluable"
    execute_on = initial
  []
  [evaluable1]
    type = ElementUOAux
    variable = evaluable1
    element_user_object = ghosting_uo1
    field_name = "evaluable"
    execute_on = initial
  []
  [evaluable2]
    type = ElementUOAux
    variable = evaluable2
    element_user_object = ghosting_uo2
    field_name = "evaluable"
    execute_on = initial
  []
  [proc]
    type = ProcessorIDAux
    variable = proc
    execute_on = initial
  []
[]

[UserObjects]
  [ghosting_uo0]
    type = ElemSideNeighborLayersGeomTester
    execute_on = initial
    element_side_neighbor_layers = 2
    rank = 0
  []
  [ghosting_uo1]
    type = ElemSideNeighborLayersGeomTester
    execute_on = initial
    element_side_neighbor_layers = 2
    rank = 1
  []
  [ghosting_uo2]
    type = ElemSideNeighborLayersGeomTester
    execute_on = initial
    element_side_neighbor_layers = 2
    rank = 2
  []
[]

[Kernels]
  [./PolycrystalKernel]
  [../]
[]

[AuxKernels]
  [./BndsCalc]
    type = BndsCalcAux
    variable = bnds
    execute_on = timestep_end
  [../]
[]

[BCs]
  [./Periodic]
    [./All]
      auto_direction = 'x y'
    [../]
  [../]
[]

[Materials]
  [./Copper]
    type = GBEvolution
    T = 500 # K
    wGB = 60 # nm
    GBmob0 = 2.5e-6 #m^4/(Js) from Schoenfelder 1997
    Q = 0.23 #Migration energy in eV
    GBenergy = 0.708 #GB energy in J/m^2
  [../]
[]

[Postprocessors]
  active = ''
  [./ngrains]
    type = FeatureFloodCount
    variable = bnds
    threshold = 0.7
  [../]
[]

[Preconditioning]
  active = ''
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 31'

  l_tol = 1.0e-4
  l_max_its = 30
  nl_max_its = 20
  nl_rel_tol = 1.0e-13
  start_time = 0.0
  num_steps = 2
  dt = 80.0

  [./Adaptivity]
    initial_adaptivity = 2
    refine_fraction = 0.7
    coarsen_fraction = 0.1
    max_h_level = 1
  [../]
[]

[Outputs]
  exodus = true
[]
