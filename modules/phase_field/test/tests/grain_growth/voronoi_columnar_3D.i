[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 40
  ny = 40
  nz = 1
  xmin = 0
  xmax = 1000
  ymin = 0
  ymax = 1000
  zmin = 0
  zmax = 25
  elem_type = HEX8
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
    rand_seed = 47
    grain_num = 4
    columnar_3D = true
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

[Executioner]
  type = Transient
  scheme = 'bdf2'
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 31'

  l_tol = 1.0e-5
  l_max_its = 15
  nl_max_its = 20
  nl_rel_tol = 1.0e-10
  start_time = 0.0
  num_steps = 1
  dt = 40.0
[]

[Outputs]
  exodus = true
[]
