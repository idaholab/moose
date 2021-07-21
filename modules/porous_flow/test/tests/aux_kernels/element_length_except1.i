# The PorousFlowElementLength is used with a nodal AuxVariable to illustrate that an error is produced
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
    type = PorousFlowElementLength
    direction = '1 0 0'
    variable = nodal_aux
  []
[]

[Executioner]
  type = Transient
[]

