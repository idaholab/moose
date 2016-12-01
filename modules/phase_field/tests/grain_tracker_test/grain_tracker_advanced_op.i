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

  parallel_type = replicated # required for advanced_op_assignment
[]

[GlobalParams]
  op_num = 8
  var_name_base = gr
[]

[Variables]
  [./PolycrystalVariables]
  [../]
[]

[ICs]
  [./PolycrystalICs]
    [./PolycrystalVoronoiIC]
      rand_seed = 1
      grain_num = 40
      advanced_op_assignment = true
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
  [./grain_tracker]
    type = GrainTracker
    threshold = 0.5
    connecting_threshold = 0.5
    remap_grains = false
    halo_level = 0
  [../]
  [./DOFs]
    type = NumDOFs
    execute_on = 'initial timestep_end'
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 0
  dt = 100.0
[]

[Outputs]
  csv = true
  print_perf_log = true
[]

[Problem]
  solve = false
[]
