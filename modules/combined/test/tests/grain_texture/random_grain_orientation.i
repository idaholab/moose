[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 40
  ny = 12
  xmax = 1000
  ymax = 300
  elem_type = QUAD4
[]

[GlobalParams]
  op_num = 2
  var_name_base = gr
[]

[Variables]
  [./PolycrystalVariables]
  [../]
[]

[ICs]
  [./PolycrystalICs]
    [./BicrystalBoundingBoxIC]
      x1 = 0
      y1 = 0
      x2 = 500
      y2 = 1000
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
  [./active_bounds_elemental]
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
    execute_on = timestep_end
  [../]
  [./unique_grains]
    type = FeatureFloodCountAux
    variable = unique_grains
    flood_counter = grain_tracker
    execute_on = 'initial timestep_begin'
    field_display = UNIQUE_REGION
  [../]
  [./var_indices]
    type = FeatureFloodCountAux
    variable = var_indices
    flood_counter = grain_tracker
    execute_on = 'initial timestep_begin'
    field_display = VARIABLE_COLORING
  [../]
  [./active_bounds_elemental]
    type = FeatureFloodCountAux
    variable = active_bounds_elemental
    field_display = ACTIVE_BOUNDS
    execute_on = 'initial timestep_begin'
    flood_counter = grain_tracker
  [../]
[]

[Modules]
  [./PhaseField]
    [./EulerAngles2RGB]
      crystal_structure = cubic
      euler_angle_provider = euler_angle_file
      grain_tracker = grain_tracker
    [../]
  [../]
[]

[Materials]
  [./Copper]
    type = GBEvolution
    block = 0
    T = 500 # K
    wGB = 75 # nm
    GBmob0 = 2.5e-6 #m^4/(Js) from Schoenfelder 1997
    Q = 0.23 #Migration energy in eV
    GBenergy = 0.708 #GB energy in J/m^2
    time_scale = 1.0e-6
  [../]
[]

[UserObjects]
  [./grain_tracker]
    type = GrainTracker
    flood_entity_type = elemental
    outputs = none
    compute_var_to_feature_map = true
    execute_on = 'initial timestep_begin'
  [../]
  [./euler_angle_file]
    type = RandomEulerAngleProvider
    grain_tracker_object = grain_tracker
    execute_on = 'initial timestep_begin'
  [../]
[]

[Preconditioning]
  [./SMP]
   type = SMP
   full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  dt = 0.2
  num_steps = 3
[]

[Outputs]
  exodus = true
  perf_graph = true
[]
