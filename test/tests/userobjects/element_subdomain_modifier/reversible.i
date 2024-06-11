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
    block_name = 'left'
    bottom_left = '0 0 0'
    top_right = '0.25 1 1'
  []
  [right]
    type = SubdomainBoundingBoxGenerator
    input = 'left'
    block_id = 2
    block_name = 'right'
    bottom_left = '0.25 0 0'
    top_right = '1 1 1'
  []
  [moving_boundary]
    type = SideSetsBetweenSubdomainsGenerator
    input = 'right'
    new_boundary = 'moving_boundary'
    primary_block = 'left'
    paired_block = 'right'
  []
[]

[UserObjects]
  [moving_circle]
    type = CoupledVarThresholdElementSubdomainModifier
    coupled_var = 'phi'
    criterion_type = BELOW
    threshold = 0
    subdomain_id = 1
    complement_subdomain_id = 2
    moving_boundaries = 'moving_boundary'
    moving_boundary_subdomain_pairs = 'left right'
    execute_on = 'INITIAL TIMESTEP_BEGIN'
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
    [InitialCondition]
      type = FunctionIC
      function = moving_circle
    []
  []
[]

[AuxKernels]
  [phi]
    type = FunctionAux
    variable = phi
    function = moving_circle
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[Executioner]
  type = Transient
  dt = 0.3
  num_steps = 3
[]

[Outputs]
  exodus = true
[]
