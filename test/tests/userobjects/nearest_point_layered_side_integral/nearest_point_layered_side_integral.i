# This input computes both a layered average and layered integral with the
# same direction, points, and number of layers. The layered integral for "bin"
# i is directly equal to the layered average for "bin" i multiplied by
# by 0.05 (side length of 1 divided by 10 layers X side length of 1 divided by 2 points).

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
[]

[AuxVariables]
  [np_layered_integral]
    order = CONSTANT
    family = MONOMIAL
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
  [np_layered_integral]
    type = SpatialUserObjectAux
    variable = np_layered_integral
    user_object = npli
    boundary = 'front'
    execute_on = timestep_end
  []
  [np_layered_average]
    type = SpatialUserObjectAux
    variable = np_layered_average
    user_object = npla
    boundary = 'front'
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
    type = NearestPointLayeredSideAverage
    direction = x
    points = '0.5 0.25 0.5
              0.5 0.75 0.5'
    num_layers = 10
    variable = u
    boundary = 'front'
  []
  [npli]
    type = NearestPointLayeredSideIntegral
    direction = x
    points = '0.5 0.25 0.5
              0.5 0.75 0.5'
    num_layers = 10
    variable = u
    boundary = 'front'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
  hide = 'dummy'
[]
