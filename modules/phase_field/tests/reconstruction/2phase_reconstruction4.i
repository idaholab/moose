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
  type = EBSDMesh
  filename = ebsd_40x40_2_phase.txt
[]

[GlobalParams]
  op_num = 8
  var_name_base = gr
[]

[AuxVariables]
  [./var_indices]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[AuxKernels]
  [./var_indices]
    type = FeatureFloodCountAux
    variable = var_indices
    flood_counter = grain_tracker
    field_display = VARIABLE_COLORING
  [../]
[]

[UserObjects]
  [./ebsd]
    type = EBSDReader
  [../]
[]

[ICs]
  [./PolycrystalICs]
    [./ReconVarIC]
      ebsd_reader = ebsd
      coloring_algorithm = bt
      phase = 2
    [../]
  [../]
[]

[Variables]
  [./PolycrystalVariables]
  [../]
[]

[Postprocessors]
  [./grain_tracker]
    type = GrainTracker
    remap_grains = false

    # These two parameters better match those in the ICs block!
    ebsd_reader = ebsd
    phase = 2
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 0
[]

[Outputs]
  exodus = true
[]
