#
# The mesh consists of two blocks.  Block 1 is from y=0 to y=2, and
# block 2 is from y=3 to y=4.  Elements are 0.25 high.  The solution
# is u = 4y.
#
# Two sets of LayeredAverage values are computed.  In both cases, four
# layers are used.  In 'unrestricted', the layers span the entire mesh.
# In 'restricted', the layers cover only block 1.
#
# For 'unrestricted', the result is a value of 2 from 0<y<1 , a value
# of 6 from 1<y<2, and a value of 14 from 3<y<4.
#
# For 'restricted', the result is a value of 1 from 0<y<0.5, a value of
# 3 from 0.5<y<1, a value of 5 from 1<y<1.5, and a value of 7 for y>1.5.
#

[Mesh]
  file = layered_average_block.e
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./restricted]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./unrestricted]
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
  [./restricted]
    type = SpatialUserObjectAux
    variable = restricted
    execute_on = timestep_end
    user_object = restricted
  [../]
  [./unrestricted]
    type = SpatialUserObjectAux
    variable = unrestricted
    execute_on = timestep_end
    user_object = unrestricted
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
    value = 8
  [../]
  [./ul]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 12
  [../]
  [./uu]
    type = DirichletBC
    variable = u
    boundary = 4
    value = 16
  [../]
[]

[UserObjects]
  [./restricted]
    type = LayeredAverage
    direction = y
    num_layers = 4
    variable = u
    execute_on = linear
    block = 1
  [../]
  [./unrestricted]
    type = LayeredAverage
    direction = y
    num_layers = 4
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
