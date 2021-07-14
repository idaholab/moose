# The PorousFlowElementLength has ill-specified direction, to illustrate that an error is produced
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
  [n]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [nodal_aux]
    type = PorousFlowElementLength
    direction = '1 0'
    variable = n
  []
[]

[Executioner]
  type = Transient
[]

