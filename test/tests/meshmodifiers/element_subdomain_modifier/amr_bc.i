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
    block_name = 'left_block'
    bottom_left = '-1 -1 0'
    top_right = '0 1 1'
  []
  [right]
    type = SubdomainBoundingBoxGenerator
    input = 'left'
    block_id = 2
    block_name = 'right_block'
    bottom_left = '0 -1 0'
    top_right = '1 1 1'
  []
  [moving_boundary]
    type = SideSetsAroundSubdomainGenerator
    input = 'right'
    block = 1
    new_boundary = 'moving_boundary'
    normal = '1 0 0'
  []
[]

[MeshModifiers]
  [moving_circle]
    type = CoupledVarThresholdElementSubdomainModifier
    coupled_var = 'phi'
    block = 2
    criterion_type = 'ABOVE'
    threshold = 0.5
    subdomain_id = 1
    moving_boundaries = 'moving_boundary'
    moving_boundary_subdomain_pairs = 'left_block right_block'
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[AuxVariables]
  [phi]
    [AuxKernel]
      type = ParsedAux
      expression = 'exp(-((x+0.5-t)^2+(y)^2)/0.25)'
      use_xyzt = true
      execute_on = 'INITIAL TIMESTEP_BEGIN'
    []
  []
[]

[Adaptivity]
  steps = 1
  marker = 'marker'
  initial_marker = 'marker'
  max_h_level = 1
  [Indicators/indicator]
    type = GradientJumpIndicator
    variable = 'phi'
  []
  [Markers]
    [efm]
      type = ErrorFractionMarker
      indicator = 'indicator'
      coarsen = 0.2
      refine = 0.5
    []
    [marker]
      type = BoundaryPreservedMarker
      preserved_boundary = 'moving_boundary'
      marker = 'efm'
    []
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = 'u'
  []
[]

[BCs]
  active = 'mbc leftright'
  [mbc]
    type = DirichletBC
    variable = 'u'
    boundary = 'moving_boundary'
    value = 1
  []
  [nbc]
    type = NeumannBC
    variable = u
    boundary = 'moving_boundary'
    value = 10
  []
  [leftright]
    type = DirichletBC
    variable = u
    boundary = 'left right'
    value = 0
  []
[]

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 5
[]

[Outputs]
  exodus = true
[]
