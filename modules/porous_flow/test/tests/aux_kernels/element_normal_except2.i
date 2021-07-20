# The PorousFlowElementNormal is used with a zero 1D_perp vector to illustrate that an error is produced
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
    type = PorousFlowElementNormal
    variable = n
    component = x
    1D_perp = '0 0 0'
  []
[]

[Executioner]
  type = Transient
[]

