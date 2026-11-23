# A block-restricted NodalVoidVolume integrates over the elements in its blocks only, so at a node
# on the boundary of its blocks it reports that node's share of the restricted block alone, not the
# node's full void volume.  Anything that consumes the volume - the geochemistry reactor, and the
# rate conversions built on it - must therefore carry the same block restriction, or it will use a
# volume that includes rock the reactor never visits.
#
# The mesh is four unit elements, x = 0 to 4, with block 1 covering x < 2.  Porosity is 0.5 in
# block 1 and 0.25 in block 0, so with linear Lagrange each element contributes porosity/2 to each
# of its two nodes:
#
#   node x=1 (interior to block 1)  0.5/2 + 0.5/2   = 0.5
#   node x=2 (on the boundary)      0.5/2           = 0.25   restricted to block 1
#   node x=2 (on the boundary)      0.5/2 + 0.25/2  = 0.375  unrestricted
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

[Outputs]
  csv = true
[]

[UserObjects]
  [nvv_block1]
    type = NodalVoidVolume
    porosity = porosity
    concentration = u
    block = 1
  []
  [nvv_all]
    type = NodalVoidVolume
    porosity = porosity
    concentration = u
  []
[]

[AuxVariables]
  [porosity]
    family = MONOMIAL
    order = CONSTANT
  []
  [vol_block1]
    block = 1
  []
  [vol_all]
  []
[]

[AuxKernels]
  [porosity]
    type = FunctionAux
    variable = porosity
    function = 'if(x<2, 0.5, 0.25)'
  []
  [vol_block1]
    type = NodalVoidVolumeAux
    variable = vol_block1
    nodal_void_volume_uo = nvv_block1
    block = 1
  []
  [vol_all]
    type = NodalVoidVolumeAux
    variable = vol_all
    nodal_void_volume_uo = nvv_all
  []
[]

[Postprocessors]
  [interior_block1]
    type = PointValue
    point = '1 0 0'
    variable = vol_block1
  []
  [boundary_block1]
    # the node's share of block 1 alone
    type = PointValue
    point = '2 0 0'
    variable = vol_block1
  []
  [boundary_all]
    # the same node's full void volume, for contrast
    type = PointValue
    point = '2 0 0'
    variable = vol_all
  []
[]
