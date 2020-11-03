[Mesh]
  [ebsd_mesh]
    type = EBSDMeshGenerator
    filename = ebsd_small.txt
  []
[]

[GlobalParams]
  op_num = 8
  var_name_base = gr
[]

[UserObjects]
  [ebsd_reader]
    type = EBSDReader
    execute_on = initial
  []
  [ebsd]
    type = PolycrystalEBSD
    coloring_algorithm = bt
    ebsd_reader = ebsd_reader
    phase = 2
    output_adjacency_matrix = true
  []
  [grain_tracker]
    type = GrainTracker
    polycrystal_ic_uo = ebsd
  []
[]

[ICs]
  [PolycrystalICs]
    [PolycrystalColoringIC]
      polycrystal_ic_uo = ebsd
    []
  []
  [void_phase]
    type = ReconPhaseVarIC
    variable = c
    ebsd_reader = ebsd_reader
    phase = 1
  []
[]

[Variables]
  [PolycrystalVariables]
  []
[]

[AuxVariables]
  #  active = 'c bnds'

  [c]
  []
  [bnds]
  []
  [ebsd_numbers]
    family = MONOMIAL
    order = CONSTANT
  []

  # Note: Not active
  [unique_grains]
    family = MONOMIAL
    order = CONSTANT
  []
  [var_indices]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Kernels]
  [PolycrystalKernel]
    c = c
  []
[]

[AuxKernels]
  #  active = 'BndsCalc'

  [BndsCalc]
    type = BndsCalcAux
    variable = bnds
    execute_on = 'initial timestep_end'
  []
  [ebsd_numbers]
    type = EBSDReaderAvgDataAux
    data_name = feature_id
    ebsd_reader = ebsd_reader
    grain_tracker = grain_tracker
    variable = ebsd_numbers
    phase = 2
    execute_on = 'initial timestep_end'
  []

  # Note: Not active
  [unique_grains]
    type = FeatureFloodCountAux
    variable = unique_grains
    flood_counter = grain_tracker
    field_display = UNIQUE_REGION
    execute_on = 'initial timestep_end'
  []
  [var_indices]
    type = FeatureFloodCountAux
    variable = var_indices
    flood_counter = grain_tracker
    field_display = VARIABLE_COLORING
    execute_on = 'initial timestep_end'
  []
[]

[Modules]
  [PhaseField]
    [EulerAngles2RGB]
      crystal_structure = cubic
      grain_tracker = grain_tracker
      euler_angle_provider = ebsd_reader
      no_grain_color = '.1 .1 .1'
      phase = 2
    []
  []
[]

[Materials]
  [bulk]
    type = GBEvolution
    block = 0
    T = 2273
    wGB = 10.0
    GBenergy = 1.58
    GBmob0 = 9.2124e-9
    Q = 2.77
    length_scale = 1.0e-6
    time_scale = 60.0
  []
[]

[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -sub_pc_type -pc_asm_overlap -ksp_grmres_restart '
  petsc_options_value = '   asm        lu            1               21'
  start_time = 0.0
  dt = 0.2
  num_steps = 1
[]

[Outputs]
  csv = true
  exodus = true
  execute_on = 'INITIAL TIMESTEP_END'
  perf_graph = true
[]
