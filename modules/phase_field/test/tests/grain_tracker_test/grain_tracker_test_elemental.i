[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 25
  ny = 25
  nz = 0
  xmax = 1000
  ymax = 1000
  zmax = 0
  elem_type = QUAD4
[]

[GlobalParams]
  op_num = 12 # Should match grain_num so we can test with FauxGrainTracker too
  var_name_base = gr
[]

[Variables]
  [./PolycrystalVariables]
  [../]
[]

[UserObjects]
  [./voronoi]
    type = PolycrystalVoronoi
    grain_num = 12 # Number of grains
    coloring_algorithm = bt # bt will assign one grain to each op if they are the same
    rand_seed = 8675
  [../]
  [./grain_tracker]
    type = GrainTracker
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
  [./unique_grains]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./var_indices]
    order = CONSTANT
    family = MONOMIAL
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
  [../]
  [./unique_grains]
    type = FeatureFloodCountAux
    variable = unique_grains
    flood_counter = grain_tracker
    field_display = UNIQUE_REGION
  [../]
  [./var_indices]
    type = FeatureFloodCountAux
    variable = var_indices
    flood_counter = grain_tracker
    field_display = VARIABLE_COLORING
  [../]
[]

[BCs]
  [./Periodic]
    [./all]
      auto_direction = 'x y'
    [../]
  [../]
[]

[Materials]
  [./CuGrGr]
    type = GBEvolution
    T = 500 # K
    wGB = 100 # nm
    GBmob0 = 2.5e-6
    Q = 0.23
    GBenergy = 0.708
    molar_volume = 7.11e-6
  [../]
[]

[Postprocessors]
  [./DOFs]
    type = NumDOFs
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 31'
  l_tol = 1.0e-4
  l_max_its = 30
  nl_max_its = 20
  nl_rel_tol = 1.0e-9
  start_time = 0.0
  num_steps = 2
  dt = 100.0
[]

[Adaptivity]
  marker = error_marker
  max_h_level = 1
  [./Markers]
    active = 'error_marker'
    [./bnds_marker]
      type = ValueThresholdMarker
      invert = true
      refine = 0.85
      coarsen = 0.975
      third_state = DO_NOTHING
      variable = bnds
    [../]
    [./error_marker]
      type = ErrorFractionMarker
      coarsen = 0.1
      indicator = bnds_error
      refine = 0.7
    [../]
  [../]
  [./Indicators]
    [./bnds_error]
      type = GradientJumpIndicator
      variable = bnds
    [../]
  [../]
[]

[Outputs]
  exodus = true
[]
