#
# In this test we set the initial condition of a set of order parameters
# by pulling out the only grains from given EBSD data file that belong to a specified phase
#

[Problem]
  type = FEProblem
  solve = false
  kernel_coverage_check = false
[]

[Mesh]
  [ebsd_mesh]
    type = EBSDMeshGenerator
    filename = ebsd_40x40_2_phase.txt
  []
[]

[GlobalParams]
  op_num = 6
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
    phase = 2
    output_adjacency_matrix = true
  []
  [grain_tracker]
    type = GrainTracker
    polycrystal_ic_uo = ebsd
    remap_grains = false
  []
[]

[AuxVariables]
  [var_indices]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [var_indices]
    type = FeatureFloodCountAux
    variable = var_indices
    flood_counter = grain_tracker
    field_display = VARIABLE_COLORING
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

[Executioner]
  type = Transient
  num_steps = 0
[]

[Outputs]
  exodus = true
[]
