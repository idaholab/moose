[Mesh]
  # uniform_refine = 4
  type = EBSDMesh
  filename = IN100_128x128.txt
[]

[GlobalParams]
  op_num = 8
  var_name_base = gr
[]

[UserObjects]
  [./ebsd]
    type = EBSDReader
  [../]
[]

[Variables]
  [./PolycrystalVariables]
  [../]
[]

[AuxVariables]
  [./bnds]
  [../]
  [./unique_grains]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./ghost_elements]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./halos]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./var_indices]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[ICs]
  [./PolycrystalICs]
    [./ReconVarIC]
      ebsd_reader = ebsd
    [../]
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
    execute_on = 'initial timestep_end'
  [../]
  [./ghost_elements]
    type = FeatureFloodCountAux
    variable = ghost_elements
    field_display = GHOSTED_ENTITIES
    execute_on = 'initial timestep_end'
    bubble_object = grain_tracker
  [../]
  [./halos]
    type = FeatureFloodCountAux
    variable = halos
    field_display = HALOS
    execute_on = 'initial timestep_end'
    bubble_object = grain_tracker
  [../]
  [./var_indices]
    type = FeatureFloodCountAux
    variable = var_indices
    execute_on = 'initial timestep_end'
    bubble_object = grain_tracker
    field_display = VARIABLE_COLORING
  [../]
  [./unique_grains]
    type = FeatureFloodCountAux
    variable = unique_grains
    execute_on = 'initial timestep_end'
    bubble_object = grain_tracker
    field_display = UNIQUE_REGION
  [../]
[]

[Materials]
  [./Copper]
    # T = 500 # K
    type = GBEvolution
    block = 0
    T = 500
    wGB = 0.6               # um
    GBmob0 = 2.5e-6         # m^4/(Js) from Schoenfelder 1997
    Q = 0.23                # Migration energy in eV
    GBenergy = 0.708        # GB energy in J/m^2
    molar_volume = 7.11e-6  # Molar volume in m^3/mol
    length_scale = 1.0e-6
    time_scale = 1.0e-6
  [../]
[]

[Postprocessors]
  [./dt]
    type = TimestepSize
  [../]
  [./n_elements]
    type = NumElems
    execute_on = 'initial timestep_end'
  [../]
  [./n_nodes]
    type = NumNodes
    execute_on = 'initial timestep_end'
  [../]
  [./DOFs]
    type = NumDOFs
  [../]
  [./grain_tracker]
    type = GrainTracker
    threshold = 0.1
    convex_hull_buffer = 0.0
    use_single_map = false
    enable_var_coloring = true
    condense_map_info = true
    connecting_threshold = 0.05
    execute_on = 'initial timestep_end'
    flood_entity_type = ELEMENTAL
    halo_level = 2
    bubble_volume_file = IN100-grn-vols.txt
    ebsd_reader = ebsd
  [../]
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

  [./TimeStepper]
    type = IterationAdaptiveDT
    cutback_factor = 0.9
    dt = 10.0
    growth_factor = 1.1
    optimal_iterations = 7
  [../]

  [./Adaptivity]
    initial_adaptivity = 2
    refine_fraction = 0.7
    coarsen_fraction = 0.1
    max_h_level = 2
    print_changed_info = true
  [../]
[]

[Outputs]
  exodus = true
  checkpoint = true
  csv = true
  [./console]
    type = Console
    max_rows = 20
    perf_log = true
    perf_log_interval = 10
  [../]
[]
