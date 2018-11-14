#
# The mesh consists of two blocks.  Block 1 has a height and width of 1 whereas
# block 2 has a height of 2 and width of 1. A gap of 1 exists between the two
# blocks in the x direction.  Elements are 0.25 high and 1 wide.  The solution
# in block 1 is u = y and block 2 is u = 4y.
#
# Two sets of LayeredAverage values are computed.  In both cases, four
# layers are used.  In 'bounding_block1', the LayeredAverage values are computed
# on block 1 using the bounds (dimensions of block 2). In 'bounding_block2',
# the LayeredAverage values are computed on block 2 using the bounds (dimensions
# of block 1).
#
# In 'bounding_block1', since the layers are defined by the dimensions of block
# 2 only two layers appear in block one.  The values in block 1 are thus:
# 0.25 for 0<y<0.5 and 0.75 for 0.5<y<1.
#
# In 'bounding_block2', since the layers are defined by the dimensions of block
# 1 four layers appear in block two. Any place over and above the top of the
# uppermost layer is included in the uppermost layer.  Therefore, the first 3
# layers are 1/4 of the height of block 1 (0.25) whereas the 4th layer has a
# height of 1/4 of block 1 (0.25) plus the additional region in block 2 outside
# the bounds of block 1 (1.0) for a total height of 1.24.
# The values in block 2 are thus:
# 0.5 from 0<y<0.25, 1.5 from 0.25<y<0.5, 2.5 from 0.5<y<0.75, and 5.5 from
# y>0.75.
#
#

[Mesh]
  file = layered_average_bounding_block.e
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./bounding_block1]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./bounding_block2]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[AuxKernels]
  [./bounding_block1]
    type = SpatialUserObjectAux
    block = 1
    variable = bounding_block1
    execute_on = timestep_end
    user_object = bounding_block1
  [../]
  [./bounding_block2]
    type = SpatialUserObjectAux
    block = 2
    variable = bounding_block2
    execute_on = timestep_end
    user_object = bounding_block2
  [../]
[]

[BCs]
  [./ll]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]
  [./lu]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
  [./ul]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  [../]
  [./uu]
    type = DirichletBC
    variable = u
    boundary = 4
    value = 8
  [../]
[]

[UserObjects]
  [./bounding_block1]
    type = LayeredAverage
    direction = y
    num_layers = 4
    variable = u
    execute_on = linear
    block = 1
    layer_bounding_block = 2
  [../]
  [./bounding_block2]
    type = LayeredAverage
    direction = y
    num_layers = 4
    block = 2
    layer_bounding_block = 1
    variable = u
    execute_on = linear
  [../]
[]

[Executioner]
  type = Transient
  dt = 1
  end_time = 1
[]

[Outputs]
  exodus = true
[]
