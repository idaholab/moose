[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 20
  nz = 2
[]

[Problem]
  type = FEProblem
  solve = false
[]

[AuxVariables]
  [layered_integral]
    order = CONSTANT
    family = MONOMIAL
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

[UserObjects]
  # the results of the layered integral are directly compared against the analytic integral
  # of sin(y) from a to b, or cos(a) - cos(b)
  [layered_integral]
    type = FunctionLayeredIntegral
    direction = y
    num_layers = 20
    function = 'sin(y)'
  []
[]

[VectorPostprocessors]
  [li]
    type = SpatialUserObjectVectorPostprocessor
    userobject = layered_integral
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
