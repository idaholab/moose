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
  [u]
    [InitialCondition]
      type = ConstantIC
      value = 1
    []
  []
  [phi]
  []

  # for the 'displaced' test only
  inactive = 'disp_x disp_y'
  [disp_x]
  []
  [disp_y]
  []
[]

[AuxKernels]
  [phi]
    type = FunctionAux
    variable = phi
    function = moving_circle
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
  [double_u]
    type = StatefulAux
    variable = u
    coupled = u
    block = 1
  []
[]

[Postprocessors]
  # for the 'subdomain_caching' test only
  active = ''
  [average]
    type = SideAverageValue
    variable = u
    boundary = bottom
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 3
[]

[Outputs]
  [out]
    type = Exodus
  []
[]
