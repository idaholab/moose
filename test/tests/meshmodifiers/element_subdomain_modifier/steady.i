[Problem]
  solve = false
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 16
    ny = 16
  []
  [left]
    type = SubdomainBoundingBoxGenerator
    input = 'gen'
    block_id = 1
    bottom_left = '0 0 0'
    top_right = '0.25 1 1'
  []
  [right]
    type = SubdomainBoundingBoxGenerator
    input = 'left'
    block_id = 2
    bottom_left = '0.25 0 0'
    top_right = '1 1 1'
  []
[]

[UserObjects]
  [moving_circle]
    type = CoupledVarThresholdElementSubdomainModifier
    coupled_var = 'phi'
    block = 2
    criterion_type = BELOW
    threshold = 0
    subdomain_id = 1
    moving_boundary_name = moving_boundary
    execute_on = 'INITIAL'
  []
[]

[Functions]
  [moving_circle]
    type = ParsedFunction
    expression = '(x-t)^2+(y)^2-0.5^2'
  []
[]

[AuxVariables]
  [phi]
  []
[]

[AuxKernels]
  [phi]
    type = FunctionAux
    variable = phi
    function = moving_circle
    execute_on = 'INITIAL'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
