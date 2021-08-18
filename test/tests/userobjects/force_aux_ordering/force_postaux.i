[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  nx = 5
  ymin = 0
  ymax = 1
  ny = 5

  allow_renumbering = false
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
    initial_condition = 1
  []
[]

[Kernels]
  [time]
    type = TimeDerivative
    variable = u
  []

  [diff]
    type = Diffusion
    variable = u
  []
[]

# spatial_uo_1/2 are executed preaux by default because spatial_uo_aux1/2 depend on them
# We force 1 to be executed postaux, so the auxkernel will use the old value, and the
# corresponding post processor, value2, will get an old value as well
[UserObjects]
  [spatial_uo_1]
    type = LayeredSideAverage
    variable = u
    direction = y
    num_layers = 3
    boundary = 'left'
    force_postaux = true
  []
  [spatial_uo_2]
    type = LayeredSideAverage
    variable = u
    direction = y
    num_layers = 3
    boundary = 'left'
  []
[]

[AuxVariables]
  [v1]
  []
  [v2]
  []
[]

[AuxKernels]
  [spatial_uo_aux_1]
     type = SpatialUserObjectAux
     variable = v1
     user_object = 'spatial_uo_1'
  []
  [spatial_uo_aux_2]
     type = SpatialUserObjectAux
     variable = v2
     user_object = 'spatial_uo_2'
  []
[]

[Postprocessors]

  [value1]
    type = NodalVariableValue
    variable = v1
    nodeid = 3
    force_preaux = true
  []
  [value2]
    type = NodalVariableValue
    variable = v2
    nodeid = 3
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  []
[]

[Executioner]
  type = Transient
  dt = 1.0
  end_time = 2.0
[]

[Outputs]
  csv = true
[]
