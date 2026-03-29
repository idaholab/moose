[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 6
  ny = 6
  nz = 6
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [layered_integral]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[AuxKernels]
  [liaux]
    type = SpatialUserObjectAux
    variable = layered_integral
    execute_on = timestep_end
    user_object = layered_integral
  []
[]

[BCs]
  [bottom]
    type = DirichletBC
    variable = u
    boundary = bottom
    value = 0
  []
  [top]
    type = DirichletBC
    variable = u
    boundary = top
    value = 1
  []
[]

[UserObjects]
  [layered_integral]
    type = LayeredIntegral
    direction = y
    bounds = '0 1'
    bound_splits = 3
    variable = u
    execute_on = linear
  []
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
