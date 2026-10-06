# A block-restricted NodalVoidVolume knows only about the nodes of the elements in its blocks, so an
# AuxKernel that queries it elsewhere is an error.  Here the NodalVoidVolume is restricted to
# block 1 but the AuxKernel reading it is not, so it is asked for a node in block 0.
[Mesh]
  [gen]
    type = CartesianMeshGenerator
    dim = 1
    dx = '1 1 1 1'
  []
  [reactive]
    type = SubdomainBoundingBoxGenerator
    input = gen
    block_id = 1
    bottom_left = '0 0 0'
    top_right = '2 0 0'
  []
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
  [nvv_block1]
    type = NodalVoidVolume
    concentration = u
    block = 1
  []
[]

[AuxVariables]
  [vol]
  []
[]

[AuxKernels]
  [vol]
    type = NodalVoidVolumeAux
    variable = vol
    nodal_void_volume_uo = nvv_block1
  []
[]
