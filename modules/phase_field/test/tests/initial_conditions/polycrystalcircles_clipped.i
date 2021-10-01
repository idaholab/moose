[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 100
  ny = 56
  nz = 0
  xmin = 80
  xmax = 200
  ymin = 0
  ymax = 112
  zmin = 0
  zmax = 0
[]

[GlobalParams]
  op_num = 6
  var_name_base = gr
[]

[Variables]
  [./PolycrystalVariables]
  [../]
[]

[AuxVariables]
  [./unique_grains]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./var_indices]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
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
[]

[UserObjects]
  [./circle_IC]
    type = PolycrystalCircles
    file_name = 'circles.txt'
    read_from_file = true
    execute_on = 'initial'
    int_width = 2
  [../]
  [./grain_tracker]
    type = GrainTracker
    remap_grains = true
    compute_halo_maps = false
    polycrystal_ic_uo = circle_IC
  [../]
[]

[ICs]
  [./PolycrystalICs]
    [./PolycrystalColoringIC]
      polycrystal_ic_uo = circle_IC
    [../]
  [../]
[]

[Kernels]
  [./dt_gr0]
    type = TimeDerivative
    variable = gr0
  [../]
  [./dt_gr1]
    type = TimeDerivative
    variable = gr1
  [../]
  [./dt_gr2]
    type = TimeDerivative
    variable = gr2
  [../]
  [./dt_gr3]
    type = TimeDerivative
    variable = gr3
  [../]
  [./dt_gr4]
    type = TimeDerivative
    variable = gr4
  [../]
  [./dt_gr5]
    type = TimeDerivative
    variable = gr5
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = PJFNK
  num_steps = 0
[]

[Outputs]
  exodus = true
  csv = false
[]
