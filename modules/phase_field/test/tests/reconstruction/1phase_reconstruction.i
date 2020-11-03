#
# In this test we set the initial condition of a set of order parameters
# by pulling out the grain data from given EBSD data file ignoring the phase completely.
#

[Problem]
  type = FEProblem
  solve = false
  kernel_coverage_check = false
[]

# The following sections are extracted in the documentation in
# moose/docs/content/modules/phase_field/ICs/EBSD.md

[Mesh]
  # Create a mesh representing the EBSD data
  [ebsd_mesh]
    type = EBSDMeshGenerator
    filename = IN100_001_28x28_Marmot.txt
  []
[]

[GlobalParams]
  # Define the number and names of the order parameters used to represent the grains
  op_num = 4
  var_name_base = gr
[]

[UserObjects]
  [ebsd_reader]
    # Read in the EBSD data. Uses the filename given in the mesh block.
    type = EBSDReader
  []
  [ebsd]
    type = PolycrystalEBSD
    coloring_algorithm = bt
    ebsd_reader = ebsd_reader
    output_adjacency_matrix = true
  []
  [grain_tracker]
    type = GrainTracker
    # For displaying HALO fields
    compute_halo_maps = true
    # Link in the ebsd userobject here so that grain tracker can extract info from it
    polycrystal_ic_uo = ebsd
  []
[]

[Variables]
  [PolycrystalVariables]
    # Create all the order parameters
    order = FIRST
    family = LAGRANGE
  []
[]

[ICs]
  [PolycrystalICs]
    [PolycrystalColoringIC]
      # Uses the data from the user object 'ebsd' to initialize the variables for all the order parameters.
      polycrystal_ic_uo = ebsd
    []
  []
[]
#ENDDOC - End of the file section that is included in the documentation. Do not change this line!

[GlobalParams]
  execute_on = 'initial'
  family = MONOMIAL
  order = CONSTANT
[]

[AuxVariables]
  [PHI1]
  []
  [PHI]
  []
  [PHI2]
  []
  [GRAIN]
  []
  [unique_grains]
  []
  [var_indices]
  []
  [halo0]
  []
  [halo1]
  []
  [halo2]
  []
  [halo3]
  []
[]

[AuxKernels]
  [phi1_aux]
    type = EBSDReaderPointDataAux
    variable = PHI1
    ebsd_reader = ebsd_reader
    data_name = 'phi1'
  []
  [phi_aux]
    type = EBSDReaderPointDataAux
    variable = PHI
    ebsd_reader = ebsd_reader
    data_name = 'phi'
  []
  [phi2_aux]
    type = EBSDReaderPointDataAux
    variable = PHI2
    ebsd_reader = ebsd_reader
    data_name = 'phi2'
  []
  [grain_aux]
    type = EBSDReaderPointDataAux
    variable = GRAIN
    ebsd_reader = ebsd_reader
    data_name = 'feature_id'
  []
  [unique_grains]
    type = FeatureFloodCountAux
    variable = unique_grains
    flood_counter = grain_tracker
    field_display = UNIQUE_REGION
  []
  [var_indices]
    type = FeatureFloodCountAux
    variable = var_indices
    flood_counter = grain_tracker
    field_display = VARIABLE_COLORING
  []
  [halo0]
    type = FeatureFloodCountAux
    variable = halo0
    map_index = 0
    field_display = HALOS
    flood_counter = grain_tracker
  []
  [halo1]
    type = FeatureFloodCountAux
    variable = halo1
    map_index = 1
    field_display = HALOS
    flood_counter = grain_tracker
  []
  [halo2]
    type = FeatureFloodCountAux
    variable = halo2
    map_index = 2
    field_display = HALOS
    flood_counter = grain_tracker
  []
  [halo3]
    type = FeatureFloodCountAux
    variable = halo3
    map_index = 3
    field_display = HALOS
    flood_counter = grain_tracker
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
