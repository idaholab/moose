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
  [./layered_integral]
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
  [./liaux]
    type = SpatialUserObjectAux
    variable = layered_integral
    boundary = right
    user_object = layered_integral
  [../]
[]

[UserObjects]
  [./layered_integral]
    type = LayeredSideIntegral
    direction = y
    num_layers = 3
    variable = u
    execute_on = linear
    boundary = right
  [../]
[]

[VectorPostprocessors]
  [int]
    type = SpatialUserObjectVectorPostprocessor
    userobject = layered_integral
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
  csv = true
[]
