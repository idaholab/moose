# Restart from a checkpoint written after step 2 of tangent_predictor_test.i. The predictor's
# accepted tangent direction should be available immediately for the final step after restart.

[Mesh]
  file = tangent_predictor_restart_cp/0002-mesh.cpa.gz
[]

[Problem]
  type = FEProblem
  extra_tag_vectors = 'load_increment'
  restart_file_base = tangent_predictor_restart_cp/0002
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
    type = Diffusion
    variable = u
  []
  [load]
    type = BodyForce
    variable = u
    function = ramp
    extra_vector_tags = 'load_increment'
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

  nl_max_its = 12
  nl_abs_tol = 1e-13
  nl_rel_tol = 1e-13
  l_tol = 1e-13

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
  file_base = tangent_predictor_restart_out
[]
