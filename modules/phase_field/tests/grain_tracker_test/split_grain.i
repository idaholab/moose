[Mesh]
  type = EBSDMesh
  filename = EBSD_split_grain.txt

  parallel_type = replicated # required for advanced_op_assignment
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
  [./ebsd_grains]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[ICs]
  [./PolycrystalICs]
    [./ReconVarIC]
      ebsd_reader = ebsd
      advanced_op_assignment = true
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
    flood_counter = grain_tracker
  [../]
  [./halos]
    type = FeatureFloodCountAux
    variable = halos
    field_display = HALOS
    execute_on = 'initial timestep_end'
    flood_counter = grain_tracker
  [../]
  [./var_indices]
    type = FeatureFloodCountAux
    variable = var_indices
    execute_on = 'initial timestep_end'
    flood_counter = grain_tracker
    field_display = VARIABLE_COLORING
  [../]
  [./unique_grains]
    type = FeatureFloodCountAux
    variable = unique_grains
    execute_on = 'initial timestep_end'
    flood_counter = grain_tracker
    field_display = UNIQUE_REGION
  [../]
  [./grain_aux]
    type = EBSDReaderPointDataAux
    variable = ebsd_grains
    ebsd_reader = ebsd
    data_name = 'feature_id'
    execute_on = 'initial timestep_end'
  [../]
[]

[Modules]
  [./PhaseField]
    [./EulerAngles2RGB]
      crystal_structure = cubic
      euler_angle_provider = ebsd
      grain_tracker = grain_tracker
    [../]
  [../]
[]

[Materials]
  [./Copper]
    # T = 500 # K
    type = GBEvolution
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
    ebsd_reader = ebsd
    compute_halo_maps = true # Only necessary for displaying HALOS
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
  num_steps = 2

  [./TimeStepper]
    type = IterationAdaptiveDT
    cutback_factor = 0.9
    dt = 10.0
    growth_factor = 1.1
    optimal_iterations = 7
  [../]
[]

[Outputs]
  exodus = true
  csv = true
  print_perf_log = true
[]
