[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 50
    xmax = 10
    ymax = 50
  []
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [v]
    order = CONSTANT
    family = MONOMIAL
  []
  [pid]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [dot]
    type = TimeDerivative
    variable = u
  []
[]

[AuxKernels]
  [intersect]
    type = FeatureFloodCountAux
    variable = v
    flood_counter = intersection
    field_display = INTERSECTS_SPECIFIED_BOUNDARY
    execute_on = 'initial timestep_end'
  []
  [pid]
    type = ProcessorIDAux
    variable = pid
  []
[]

[ICs]
  [v]
    type = BoundingBoxIC
    variable = u
    inside = 1
    outside = 0
    x1 = 3
    x2 = 7
    y1 = 0
    y2 = 45
  []
[]

[Postprocessors]
  [intersection]
    type = FeatureFloodCount
    variable = u
    threshold = 0.3
    specified_boundaries = bottom
    compute_var_to_feature_map = true
    execute_on = 'initial timestep_end'
  []
  [vint]
    type = ElementIntegralVariablePostprocessor
    variable = v
  []
[]

[Executioner]
  type = Transient
  dt = 0.01
  num_steps = 2
[]

[Outputs]
  execute_on = 'timestep_end'
  csv = true
[]
