#
# In this test , which is set up similarly to 2phase_reconstruction_test2.i
# we demonstrate that the feature numbers in teh EBSD file can be chosen arbitrarily.
# There is no need for then to start at a certain index or even to be contiguous!
# The EBSDReaderPointDataAux AuxKernel outputs the original feature IDs (grain numbers)
#

[Problem]
  type = FEProblem
  solve = false
  kernel_coverage_check = false
[]

[Mesh]
  type = EBSDMesh
  filename = Renumbered.txt
[]

[GlobalParams]
  op_num = 3
  var_name_base = gr
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
      phase = 1
    [../]
  [../]
[]

[AuxVariables]
  [./GRAIN]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[AuxKernels]
  [./grain_aux]
    type = EBSDReaderPointDataAux
    variable = GRAIN
    ebsd_reader = ebsd
    data_name = 'feature_id'
    execute_on = 'initial'
  [../]
[]

[Variables]
  [./PolycrystalVariables]
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 0
[]

[Outputs]
  exodus = true
[]
