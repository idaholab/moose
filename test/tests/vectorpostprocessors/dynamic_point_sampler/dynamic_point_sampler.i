[Mesh]
  type = GeneratedMesh
  nx = 5
  ny = 5
  dim = 2
[]

[Variables]
  [u]
  []
[]

[Functions]
  [forcing_func]
    type = ParsedFunction
    expression = alpha*alpha*pi*pi*sin(alpha*pi*x)
    symbol_names = 'alpha'
    symbol_values = '4'
  []

  [u_func]
    type = ParsedGradFunction
    value = sin(alpha*pi*x)
    grad_x   = alpha*pi*sin(alpha*pi*x)
    symbol_names = 'alpha'
    symbol_values = '4'
  []
[]

[Kernels]
  [diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
  []

  [forcing]
    type = BodyForce
    variable = u
    function = forcing_func
  []

  [time]
    type = TimeDerivative
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = 'u'
    boundary = 'left'
    value = 0
  []

  [right]
    type = DirichletBC
    variable = 'u'
    boundary = 'right'
    value = 0
  []
[]

[Executioner]
  type = Transient
  num_steps = 7
  dt = 0.1

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Postprocessors]
  [l2_error]
    type = ElementL2Error
    variable = u
    function = u_func
  []

  [dofs]
    type = NumDOFs
  []
[]

[Adaptivity]
  max_h_level = 3
  marker = error
  [Indicators]
    [jump]
      type = GradientJumpIndicator
      variable = u
    []
  []
  [Markers]
    [error]
      type = ErrorFractionMarker
      indicator = jump
      coarsen = 0.1
      refine = 0.3
    []
  []
[]

[VectorPostprocessors]
  [dynamic_line_sampler]
    type = DynamicPointValueSampler
    variable = u
    start_point = '0 0.5 0'
    end_point = '1 0.5 0'
    num_points = 6
    sort_by = x
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  csv = true
[]
