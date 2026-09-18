  [Mesh]
    type = GeneratedMesh
    dim = 1
    nx = 1
  []

  [Variables]
    [./u]
    [../]
  []

  [Kernels]
    [./diff]
      type = Diffusion
      variable = u
    [../]
  []

  [BCs]
    [./left]
      type = DirichletBC
      variable = u
      boundary = left
      value = 0
    [../]
    [./right]
      type = DirichletBC
      variable = u
      boundary = right
      value = 1
    [../]
  []

  [Postprocessors]
    # A controllable constant Real; used by the ParsedPostprocessor
    [./c]
      type = ConstantPostprocessor
      value = 1.0
    [../]

    # ParsedPostprocessor whose expression is now controllable.
    # It uses the constant postprocessor 'c' and time 't'.
    [./parsed]
      type = ParsedPostprocessor
      expression = 'c + t'
      pp_names = 'c'
      use_t = true
    [../]
  []

  [Controls/web_server]
      type = WebServerControl
      execute_on = 'initial timestep_begin'
      initial_client_timeout = 5
      client_timeout = 5
  []

  [Executioner]
    type = Transient
    num_steps = 2
    dt = 1.0
    nl_abs_tol = 1e-8
    solve_type = PJFNK
  []

  [Outputs]
    csv = true
    execute_on = 'timestep_end'
  []
