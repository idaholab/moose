# Exception test: the nodal void volume AuxVariable is a constant monomial, ooops!
[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [u]
    type = Diffusion
    variable = u
  []
[]

[Executioner]
  type = Transient
  end_time = 1
[]

[UserObjects]
  [nodal_void_volume]
    type = NodalVoidVolume
    porosity = 1
  []
[]

[AuxVariables]
  [vol]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [vol]
    type = NodalVoidVolumeAux
    variable = vol
    nodal_void_volume_uo = nodal_void_volume
  []
[]
