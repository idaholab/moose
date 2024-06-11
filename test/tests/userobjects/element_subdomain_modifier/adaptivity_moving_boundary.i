[Problem]
  solve = false
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -1
    xmax = 1
    ymin = -1
    ymax = 1
    nx = 16
    ny = 16
  []
  [left]
    type = SubdomainBoundingBoxGenerator
    input = 'gen'
    block_id = 1
    block_name = 'left'
    bottom_left = '-1 -1 0'
    top_right = '0 1 1'
  []
  [right]
    type = SubdomainBoundingBoxGenerator
    input = 'left'
    block_id = 2
    block_name = 'right'
    bottom_left = '0 -1 0'
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
    debug_element_id = 652
    type = CoupledVarThresholdElementSubdomainModifier
    coupled_var = 'phi'
    block = 2
    criterion_type = ABOVE
    threshold = 0.5
    subdomain_id = 1
    moving_boundaries = 'moving_boundary'
    moving_boundary_subdomain_pairs = 'left right'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Functions]
  [moving_gauss]
    type = ParsedFunction
    value = 'exp(-((x+0.5-t)^2+(y)^2)/0.25)'
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
    function = moving_gauss
    execute_on = 'INITIAL TIMESTEP_BEGIN TIMESTEP_END'
  []
[]

[Adaptivity]
  steps = 1
  marker = marker
  initial_marker = marker
  max_h_level = 1
  [Indicators]
    [indicator]
      type = GradientJumpIndicator
      variable = phi
    []
  []
  [Markers]
    [marker]
      type = ErrorFractionMarker
      indicator = indicator
      coarsen = 0.2
      refine = 0.5
    []
    # [marker]
    #   type = BoundaryPreservedMarker
    #   preserved_boundary = moving_boundary
    #   marker = 'efm'
    # []
  []
[]

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 10
[]

[Outputs]
  exodus = true
[]
