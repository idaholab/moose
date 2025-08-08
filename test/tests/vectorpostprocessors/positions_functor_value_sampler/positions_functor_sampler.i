[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Problem]
  solve = false
[]

[AuxVariables]
  [fv]
    type = MooseVariableFVReal
    initial_condition = 3
  []
[]

[Functions]
  [fx]
    type = ParsedFunction
    expression = 'x'
  []
  [fy]
    type = ParsedFunction
    expression = 'y'
  []
[]

[Positions]
  [pos]
    type = InputPositions
    positions = '0.11 0.11 0
                 0.21 0.15 0
                 0.11 0.21 0'
  []
[]

[VectorPostprocessors]
  [point_sample]
    type = PositionsFunctorValueSampler
    functors = 'fv 2 fx fy'
    positions = 'pos'
    sort_by = id
    execute_on = TIMESTEP_END
    # the finite volume variable is discontinuous at faces
    # Note that we are not sampling on faces so it does not matter,
    # we could set it to 'false' to have less checks when sampling
    discontinuous = true
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  execute_on = 'timestep_end'
  csv = true
[]
