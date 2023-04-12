[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
[]

[Problem]
  type = ReferenceResidualProblem
  # reference_vector = 'absref'
  # extra_tag_vectors = 'absref'
[]

[Variables]
  [u][]
  [v]
    scaling = 1e-6
  []
[]

[Functions]
  [ramp]
    type = ParsedFunction
    expression = 'if(t < 5, t - 5, 0) * x'
  []
[]

[Kernels]
  [u_dt]
    type = TimeDerivative
    variable = u
  []
  [u_coupled_rx]
    type = CoupledForce
    variable = u
    v = v
    coef = 1
  []

  [v_dt]
    type = TimeDerivative
    variable = v
  []
  [v_neg_force]
    type = BodyForce
    variable = v
    value = ${fparse -1 / 2}
    function = ramp
  []
  [v_force]
    type = BodyForce
    variable = v
    value = 1
    function = ramp
  []
[]

[Postprocessors]
  [u_avg]
    type = ElementAverageValue
    variable = u
    execute_on = 'TIMESTEP_END INITIAL'
  []
  [v_avg]
    type = ElementAverageValue
    variable = v
    execute_on = 'TIMESTEP_END INITIAL'
  []
  [timestep]
    type = TimePostprocessor
    outputs = 'none'
  []
  [v_old]
    type = ElementAverageValue
    variable = v
    execute_on = TIMESTEP_BEGIN
    outputs = none
  []
  [u_old]
    type = ElementAverageValue
    variable = u
    execute_on = TIMESTEP_BEGIN
    outputs = none
  []
  [v_exact]
    type = ParsedPostprocessor
    pp_names = 'timestep v_old'
    function = 't := if(timestep > 5, 5, timestep); (t^2 - 9 * t) / 8'
  []
  [u_exact]
    type = ParsedPostprocessor
    pp_names = 'u_old v_exact'
    function = 'u_old + v_exact'
  []
[]


[Executioner]
  type = Transient
  petsc_options = '-snes_converged_reason'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  line_search = none
  num_steps = 10
  nl_rel_tol = 1e-06
  verbose = true
[]

[Outputs]
  csv = true
  perf_graph = true
[]
