# The PorousFlowElementNormal is used with a nodal AuxVariable to illustrate that an error is produced
[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 1
  []
[]

[Variables]
  [dummy]
  []
[]

[Kernels]
  [dummy]
    type = Diffusion
    variable = dummy
  []
[]

[AuxVariables]
  [nodal_aux]
  []
[]

[AuxKernels]
  [nodal_aux]
    type = PorousFlowElementNormal
    variable = nodal_aux
    component = x
  []
[]

[Executioner]
  type = Transient
[]

