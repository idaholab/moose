#
# In this test we set the initial condition of a set of order parameters
# by pulling out the grain data from given EBSD data file ignoring the phase completely.
#

[Problem]
  type = FEProblem
  solve = false
  kernel_coverage_check = false
[]

[Mesh]
  type = EBSDMesh
  filename = IN100_001_28x28_Marmot.txt
[]

[GlobalParams]
  op_num = 8
  var_name_base = gr
  execute_on = 'initial'
  family = MONOMIAL
  order = CONSTANT
[]

[UserObjects]
  [./ebsd]
    type = EBSDReader
  [../]
  [./grain_tracker]
    type = GrainTracker
    ebsd_reader = ebsd
    compute_halo_maps = true # For displaying HALO fields
  [../]
[]

[ICs]
  [./PolycrystalICs]
    [./ReconVarIC]
      ebsd_reader = ebsd
    [../]
  [../]
[]

[Variables]
  [./PolycrystalVariables]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./PHI1]
  [../]
  [./PHI]
  [../]
  [./PHI2]
  [../]
  [./GRAIN]
  [../]
  [./unique_grains]
  [../]
  [./var_indices]
  [../]
  [./halo0]
  [../]
  [./halo1]
  [../]
  [./halo2]
  [../]
  [./halo3]
  [../]
  [./halo4]
  [../]
  [./halo5]
  [../]
  [./halo6]
  [../]
  [./halo7]
  [../]
[]

[AuxKernels]
  [./phi1_aux]
    type = EBSDReaderPointDataAux
    variable = PHI1
    ebsd_reader = ebsd
    data_name = 'phi1'
  [../]
  [./phi_aux]
    type = EBSDReaderPointDataAux
    variable = PHI
    ebsd_reader = ebsd
    data_name = 'phi'
  [../]
  [./phi2_aux]
    type = EBSDReaderPointDataAux
    variable = PHI2
    ebsd_reader = ebsd
    data_name = 'phi2'
  [../]
  [./grain_aux]
    type = EBSDReaderPointDataAux
    variable = GRAIN
    ebsd_reader = ebsd
    data_name = 'feature_id'
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

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
