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

[Materials]
  [stateful]
    type = StatefulMaterial
    initial_diffusivity = 0.5
    multiplier = 2
    block = 1
    outputs = exodus
  []
  [non_stateful]
    type = GenericConstantMaterial
    prop_names = 'diffusivity'
    prop_values = '0.5'
    block = 2
    outputs = exodus
  []
[]

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 3
[]

[Outputs]
  exodus = true
[]
