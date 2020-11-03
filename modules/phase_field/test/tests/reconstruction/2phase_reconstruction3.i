#
# In this test , which is set up similarly to 2phase_reconstruction_test2.i
# we demonstrate that the feature numbers in the EBSD file can be chosen arbitrarily.
# There is no need for then to start at a certain index or even to be contiguous!
# The EBSDReaderPointDataAux AuxKernel outputs the original feature IDs (grain numbers)
#

[Problem]
  type = FEProblem
  solve = false
  kernel_coverage_check = false
[]

[Mesh]
  [ebsd_mesh]
    type = EBSDMeshGenerator
    filename = Renumbered.txt
  []
[]

[GlobalParams]
  op_num = 2
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
    phase = 1
    output_adjacency_matrix = true
  []
[]

[ICs]
  [PolycrystalICs]
    [PolycrystalColoringIC]
      polycrystal_ic_uo = ebsd
    []
  []
[]

[AuxVariables]
  [GRAIN]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [grain_aux]
    type = EBSDReaderPointDataAux
    variable = GRAIN
    ebsd_reader = ebsd_reader
    data_name = 'feature_id'
    execute_on = 'initial'
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
