# Nonlinear conductivity keeps the tangent predictor as an approximation rather than an exact
# one-step prediction.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[Problem]
  type = FEProblem
  extra_tag_vectors = 'load_increment'
[]

[Functions]
  [ramp]
    type = ParsedFunction
    expression = 't'
  []
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [diffusion]
    type = MatDiffusion
    variable = u
    diffusivity = conductivity
  []
  [load]
    type = BodyForce
    variable = u
    function = ramp
    extra_vector_tags = 'load_increment'
  []
[]

[Materials]
  [conductivity]
    type = DerivativeParsedMaterial
    property_name = conductivity
    expression = '1 + 0.1*u'
    coupled_variables = 'u'
    derivative_order = 1
  []
[]

[BCs]
  [boundary]
    type = DirichletBC
    variable = u
    boundary = 'left right top bottom'
    value = 0
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  line_search = none

  nl_max_its = 15
  nl_abs_tol = 1e-13
  nl_rel_tol = 1e-13
  l_tol = 1e-13

  start_time = 0
  dt = 0.25
  end_time = 0.75

  [Predictor]
    type = TangentPredictor
    scale = 1
    load_vector_tag = load_increment
  []
[]

[Postprocessors]
  [dt]
    type = TimestepSize
  []
  [initial_residual]
    type = Residual
    residual_type = INITIAL
  []
  [final_residual]
    type = Residual
    residual_type = FINAL
  []
  [num_nonlinear_iterations]
    type = NumNonlinearIterations
  []
  [num_linear_iterations]
    type = NumLinearIterations
  []
  [u_average]
    type = ElementAverageValue
    variable = u
    execute_on = 'LINEAR TIMESTEP_END'
  []
[]

[Outputs]
  csv = true
[]
