[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 6
  ny = 6
  nz = 6
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./layered_side_average]
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

[BCs]
  [./bottom]
    type = DirichletBC
    variable = u
    boundary = bottom
    value = 0
  [../]
  [./top]
    type = DirichletBC
    variable = u
    boundary = top
    value = 1
  [../]
[]

[AuxKernels]
  [./lsia]
    type = SpatialUserObjectAux
    variable = layered_side_average
    boundary = right
    user_object = layered_side_average
  [../]
[]

[UserObjects]
  [./layered_side_average]
    type = LayeredSideAverage
    direction = y
    num_layers = 3
    variable = u
    execute_on = linear
    boundary = right
  [../]
[]

[VectorPostprocessors]
  [avg]
    type = SpatialUserObjectVectorPostprocessor
    userobject = layered_side_average
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
  csv = true
[]
