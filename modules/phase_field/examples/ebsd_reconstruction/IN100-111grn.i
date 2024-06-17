[Mesh]
  [ebsd_mesh]
    type = EBSDMeshGenerator
    filename = IN100_120x120.txt
    pre_refine = 2
  []
[]

[GlobalParams]
  op_num = 8
  var_name_base = gr
[]

[UserObjects]
  [ebsd_reader]
    type = EBSDReader
  []
  [ebsd]
    type = PolycrystalEBSD
    coloring_algorithm = bt
    ebsd_reader = ebsd_reader
    enable_var_coloring = true
  []
  [grain_tracker]
    type = GrainTracker
    flood_entity_type = ELEMENTAL
    compute_halo_maps = true # For displaying HALO fields
    polycrystal_ic_uo = ebsd
  []
[]

[ICs]
  [PolycrystalICs]
    [PolycrystalColoringIC]
      polycrystal_ic_uo = ebsd
    []
  []
[]

[Variables]
  [PolycrystalVariables]
  []
[]

[AuxVariables]
  [bnds]
  []
  [unique_grains_ic]
    order = CONSTANT
    family = MONOMIAL
  []
  [unique_grains]
    order = CONSTANT
    family = MONOMIAL
  []
  [ghost_elements]
    order = CONSTANT
    family = MONOMIAL
  []
  [halos]
    order = CONSTANT
    family = MONOMIAL
  []
  [var_indices_ic]
    order = CONSTANT
    family = MONOMIAL
  []
  [var_indices]
    order = CONSTANT
    family = MONOMIAL
  []
  [ebsd_grains]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Kernels]
  [PolycrystalKernel]
  []
[]

[AuxKernels]
  [BndsCalc]
    type = BndsCalcAux
    variable = bnds
    execute_on = 'initial timestep_end'
  []
  [ghost_elements]
    type = FeatureFloodCountAux
    variable = ghost_elements
    field_display = GHOSTED_ENTITIES
    execute_on = 'initial timestep_end'
    flood_counter = grain_tracker
  []
  [halos]
    type = FeatureFloodCountAux
    variable = halos
    field_display = HALOS
    execute_on = 'initial timestep_end'
    flood_counter = grain_tracker
  []
  [var_indices_ic]
    type = FeatureFloodCountAux
    variable = var_indices_ic
    execute_on = 'initial'
    flood_counter = ebsd
    field_display = VARIABLE_COLORING
  []
  [unique_grains_ic]
    type = FeatureFloodCountAux
    variable = unique_grains_ic
    execute_on = 'initial'
    flood_counter = ebsd
    field_display = UNIQUE_REGION
  []
  [var_indices]
    type = FeatureFloodCountAux
    variable = var_indices
    execute_on = 'initial timestep_end'
    flood_counter = grain_tracker
    field_display = VARIABLE_COLORING
  []
  [unique_grains]
    type = FeatureFloodCountAux
    variable = unique_grains
    execute_on = 'initial timestep_end'
    flood_counter = grain_tracker
    field_display = UNIQUE_REGION
  []
  [grain_aux]
    type = EBSDReaderPointDataAux
    variable = ebsd_grains
    ebsd_reader = ebsd_reader
    data_name = 'feature_id'
    execute_on = 'initial timestep_end'
  []
[]

[Modules]
  [PhaseField]
    [EulerAngles2RGB]
      crystal_structure = cubic
      euler_angle_provider = ebsd_reader
      grain_tracker = grain_tracker
    []
  []
[]

[Materials]
  [Copper]
    # T = 500 # K
    type = GBEvolution
    T = 500
    wGB = 0.6 # um
    GBmob0 = 2.5e-6 # m^4/(Js) from Schoenfelder 1997
    Q = 0.23 # Migration energy in eV
    GBenergy = 0.708 # GB energy in J/m^2
    molar_volume = 7.11e-6 # Molar volume in m^3/mol
    length_scale = 1.0e-6
    time_scale = 1.0e-6
  []
[]

[Postprocessors]
  [dt]
    type = TimestepSize
  []
  [n_elements]
    type = NumElements
    execute_on = 'initial timestep_end'
  []
  [n_nodes]
    type = NumNodes
    execute_on = 'initial timestep_end'
  []
  [DOFs]
    type = NumDOFs
  []
[]

[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = PJFNK

  petsc_options_iname = '-pc_type -pc_hypre_type -pc_hypre_boomeramg_strong_threshold'
  petsc_options_value = 'hypre    boomeramg      0.7'

  l_tol = 1.0e-4
  l_max_its = 20
  nl_max_its = 20
  nl_rel_tol = 1.0e-8

  start_time = 0.0
  num_steps = 30

  [TimeStepper]
    type = IterationAdaptiveDT
    cutback_factor = 0.9
    dt = 10.0
    growth_factor = 1.1
    optimal_iterations = 7
  []

  [Adaptivity]
    initial_adaptivity = 2
    refine_fraction = 0.7
    coarsen_fraction = 0.1
    max_h_level = 2
  []
[]

[Outputs]
  exodus = true
  checkpoint = true
  perf_graph = true
[]
