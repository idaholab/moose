# This off-diagonal two-variable system has an invertible full tangent but a zero diagonal. It is
# used to exercise the diagonal predictor's non-finite direction diagnostic.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 4
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
  []
  [v]
  []
[]

[Kernels]
  [u_coupled]
    type = CoupledForce
    variable = u
    v = v
  []
  [u_load]
    type = BodyForce
    variable = u
    function = ramp
    extra_vector_tags = 'load_increment'
  []
  [v_coupled]
    type = CoupledForce
    variable = v
    v = u
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  line_search = none

  # Pin pc_type=none: this test intentionally has a zero Jacobian diagonal, and a direct
  # factorization would fail before the predictor's diagonal-mode diagnostic is reached.
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'none'

  nl_max_its = 8
  nl_abs_tol = 1e-13
  nl_rel_tol = 1e-13

  start_time = 0
  dt = 0.5
  num_steps = 2

  [Predictor]
    type = TangentPredictor
    scale = 1
    load_vector_tag = load_increment
    use_diagonal_approximation = true
  []
[]

[Postprocessors]
  [initial_residual]
    type = Residual
    residual_type = INITIAL
  []
  [final_residual]
    type = Residual
    residual_type = FINAL
  []
[]

[Outputs]
  csv = true
[]
