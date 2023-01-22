[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 10
  ny = 10
  nz = 10
[]

[Variables]
  [dummy]
  []
[]

[Kernels]
  [diffusion]
    type = Diffusion
    variable = dummy
  []
[]

[AuxVariables]
  [u]
  []
  [np_layered_average]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [u]
    type = FunctionAux
    variable = u
    function = u
  []
  [np_layered_average]
    type = SpatialUserObjectAux
    variable = np_layered_average
    user_object = npla
    execute_on = timestep_end
  []
[]

[Functions]
  [u]
    type = ParsedFunction
    expression = 'x+2*y+3*z'
  []
[]

[UserObjects]
  [npla]
    type = NearestPointLayeredAverage
    direction = x
    points = '0.5 0.25 0.25
              0.5 0.75 0.25
              0.5 0.25 0.75
              0.5 0.75 0.75'
    num_layers = 3
    variable = u
  []
[]

[VectorPostprocessors]
  [vpp]
    type = SpatialUserObjectVectorPostprocessor
    userobject = npla
    points_file = points.txt
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
  execute_on = 'final'
  hide = 'dummy'
[]
