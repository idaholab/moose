#
# In this test we set the initial condition of a set of order parameters
# by pulling out the only grains from given EBSD data file that belong to a specified phase
#

[Problem]
  type = FEProblem
  solve = false
  kernel_coverage_check = false
[]

# The following sections are extracted in the documentation in
# moose/docs/content/modules/phase_field/ICs/EBSD.md

[Mesh]
  [ebsd_mesh]
    type = EBSDMeshGenerator
    filename = Ti_2Phase_28x28_ebsd.txt
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

[Variables]
  [PolycrystalVariables]
  []
[]

[ICs]
  [PolycrystalICs]
    [PolycrystalColoringIC]
      # select only data for phase 1 from the EBSD file
      polycrystal_ic_uo = ebsd
    []
  []
[]
#ENDDOC - End of the file section that is included in the documentation. Do not change this line!

[Executioner]
  type = Transient
  num_steps = 0
[]

[Outputs]
  exodus = true
[]
