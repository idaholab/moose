[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 35
  ny = 35
  xmax = 1000
  ymax = 1000
  elem_type = QUAD4

  parallel_type = replicated # Periodic BCs
[]

[GlobalParams]
  op_num = 8  # Number of order parameters used
  var_name_base = 'gr'  # Base name of grains
[]

[Variables]
  [./PolycrystalVariables]
  [../]
[]

[UserObjects]
  [./voronoi]
    type = PolycrystalVoronoi
    rand_seed = 12
    grain_num = 15      # Number of grains
    coloring_algorithm = bt
  [../]
  [./grain_tracker]
    type = GrainTracker
    threshold = 0.2
    connecting_threshold = 0.08
    flood_entity_type = ELEMENTAL
    compute_halo_maps = true    # Only necessary for displaying HALOS
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
  [../]
  [./unique_grains]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./var_indices]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./ghost_regions]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./halos]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./proc_id]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./halo0]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./halo1]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./halo2]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./halo3]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./halo4]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./halo5]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./halo6]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./halo7]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./PolycrystalKernel]
  [../]
[]

[AuxKernels]
  [./bnds_aux]
    type = BndsCalcAux
    variable = bnds
    execute_on = 'initial timestep_end'
  [../]
  [./unique_grains]
    type = FeatureFloodCountAux
    variable = unique_grains
    flood_counter = grain_tracker
    field_display = UNIQUE_REGION
    execute_on = 'initial timestep_end'
  [../]
  [./var_indices]
    type = FeatureFloodCountAux
    variable = var_indices
    flood_counter = grain_tracker
    field_display = VARIABLE_COLORING
    execute_on = 'initial timestep_end'
  [../]
  [./ghosted_entities]
    type = FeatureFloodCountAux
    variable = ghost_regions
    flood_counter = grain_tracker
    field_display = GHOSTED_ENTITIES
    execute_on = 'initial timestep_end'
  [../]
  [./halos]
    type = FeatureFloodCountAux
    variable = halos
    flood_counter = grain_tracker
    field_display = HALOS
    execute_on = 'initial timestep_end'
  [../]
  [./proc_id]
    type = ProcessorIDAux
    variable = proc_id
  [../]
  [./halo0]
    type = FeatureFloodCountAux
    variable = halo0
    map_index = 0
    field_display = HALOS
    flood_counter = grain_tracker
  [../]
  [./halo1]
    type = FeatureFloodCountAux
    variable = halo1
    map_index = 1
    field_display = HALOS
    flood_counter = grain_tracker
  [../]
  [./halo2]
    type = FeatureFloodCountAux
    variable = halo2
    map_index = 2
    field_display = HALOS
    flood_counter = grain_tracker
  [../]
  [./halo3]
    type = FeatureFloodCountAux
    variable = halo3
    map_index = 3
    field_display = HALOS
    flood_counter = grain_tracker
  [../]
  [./halo4]
    type = FeatureFloodCountAux
    variable = halo4
    map_index = 4
    field_display = HALOS
    flood_counter = grain_tracker
  [../]
  [./halo5]
    type = FeatureFloodCountAux
    variable = halo5
    map_index = 5
    field_display = HALOS
    flood_counter = grain_tracker
  [../]
  [./halo6]
    type = FeatureFloodCountAux
    variable = halo6
    map_index = 6
    field_display = HALOS
    flood_counter = grain_tracker
  [../]
  [./halo7]
    type = FeatureFloodCountAux
    variable = halo7
    map_index = 7
    field_display = HALOS
    flood_counter = grain_tracker
  [../]
[]

[BCs]
  [./Periodic]
    [./top_bottom]
      auto_direction = 'x y'
    [../]
  [../]
[]

[Materials]
  [./CuGrGr]
    type = GBEvolution
    T = '450'
    wGB = 125
    GBmob0 = 2.5e-6
    Q = 0.23
    GBenergy = 0.708
  [../]
[]

[Postprocessors]
  [./dt]
    type = TimestepSize
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart -mat_mffd_type'
  petsc_options_value = 'hypre boomeramg 101 ds'
  l_max_its = 30
  l_tol = 1e-4
  nl_max_its = 40
  nl_rel_tol = 1e-11
  dt = 25
  num_steps = 1
[]

[Outputs]
  exodus = true  # Exodus file will be outputted
[]
