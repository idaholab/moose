################################################################################
## Marvel surrogate SCM model                                                 ##
## Marvel SCM geometry setup simulation                                       ##
## POC : Vasileios Kyriakopoulos, vasileios.kyriakopoulos@inl.gov             ##
################################################################################
heated_length = 0.51
[TriSubChannelMesh]
  [subchannel]
    type = TriSubChannelMeshGenerator
    nrings = 4
    n_cells = 40
    flat_to_flat = 0.22 #0.21667
    heated_length = ${heated_length}
    unheated_length_entry = 0.2
    unheated_length_exit = 0.2
    pin_diameter = 0.03269
    pitch = 0.0346514
    dwire = 0.0
    hwire = 0.0
    spacer_z = '0.0'
    spacer_k = '0.0'
  []
[]

[AuxVariables]
  [S]
    block = subchannel
  []
  [w_perim]
    block = subchannel
  []
[]


[Problem]
    type = NoSolveProblem
[]

[ICs]
  [S_IC]
    type = MarvelTriFlowAreaIC
    variable = S
  []

  [w_perim_IC]
    type = MarvelTriWettedPerimIC
    variable = w_perim
  []
[]


[Outputs]
  exodus = true
[]

[Executioner]
  type = Steady
[]
