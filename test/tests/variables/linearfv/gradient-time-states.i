[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 4
  []
[]

[Problem]
  linear_sys_names = 'u_sys'
[]

[Variables]
  [u]
    type = MooseLinearVariableFVReal
    solver_sys = 'u_sys'
  []
  [v]
    type = MooseLinearVariableFVReal
    solver_sys = 'u_sys'
  []
[]

[AuxVariables]
  [aux_u]
    type = MooseLinearVariableFVReal
  []
[]

[ICs]
  [u]
    type = FunctionIC
    variable = u
    function = initial_solution
  []
  [aux_u]
    type = FunctionIC
    variable = aux_u
    function = initial_solution
  []
  [v]
    type = FunctionIC
    variable = v
    function = initial_solution
  []
[]

[AuxKernels]
  [set_aux_u]
    type = FunctionAux
    variable = aux_u
    function = exact_solution
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[LinearFVKernels]
  [time]
    type = LinearFVTimeDerivative
    variable = u
  []
  [source]
    type = LinearFVSource
    variable = u
    source_density = source
  []
  [time_v]
    type = LinearFVTimeDerivative
    variable = v
  []
  [source_v]
    type = LinearFVSource
    variable = v
    source_density = source
  []
[]

[Functions]
  [initial_solution]
    type = ParsedFunction
    expression = x
  []
  [exact_solution]
    type = ParsedFunction
    expression = '(1+t)*x'
  []
  [source]
    type = ParsedFunction
    expression = x
  []
[]

[Postprocessors]
  [u_current]
    type = LinearFVGradientStateTest
    variable = u
    initial_oldest_gradient_state = 0
    oldest_gradient_state = 3
    state = 0
    element_id = 1
    execute_on = TIMESTEP_END
  []
  [u_old]
    type = LinearFVGradientStateTest
    variable = u
    oldest_gradient_state = 1
    state = 1
    element_id = 1
    execute_on = TIMESTEP_END
  []
  [u_older]
    type = LinearFVGradientStateTest
    variable = u
    oldest_gradient_state = 2
    state = 2
    element_id = 1
    execute_on = TIMESTEP_END
  []
  [u_oldest]
    type = LinearFVGradientStateTest
    variable = u
    oldest_gradient_state = 3
    state = 3
    element_id = 1
    execute_on = TIMESTEP_END
  []
  [u_face_current]
    type = LinearFVGradientStateTest
    variable = u
    oldest_gradient_state = 1
    state = 0
    face = true
    execute_on = TIMESTEP_END
  []
  [u_face_old]
    type = LinearFVGradientStateTest
    variable = u
    oldest_gradient_state = 1
    state = 1
    face = true
    execute_on = TIMESTEP_END
  []
  [aux_current]
    type = LinearFVGradientStateTest
    variable = aux_u
    oldest_gradient_state = 3
    state = 0
    element_id = 1
    execute_on = TIMESTEP_END
  []
  [aux_old]
    type = LinearFVGradientStateTest
    variable = aux_u
    oldest_gradient_state = 3
    state = 1
    element_id = 1
    execute_on = TIMESTEP_END
  []
  [aux_older]
    type = LinearFVGradientStateTest
    variable = aux_u
    oldest_gradient_state = 3
    state = 2
    element_id = 1
    execute_on = TIMESTEP_END
  []
  [aux_oldest]
    type = LinearFVGradientStateTest
    variable = aux_u
    oldest_gradient_state = 3
    state = 3
    element_id = 1
    execute_on = TIMESTEP_END
  []
  [v_shared_oldest]
    type = LinearFVGradientStateTest
    variable = v
    oldest_gradient_state = 0
    state = 3
    element_id = 1
    execute_on = TIMESTEP_END
  []
[]

[Executioner]
  type = Transient
  system_names = u_sys
  scheme = implicit-euler
  dt = 1
  num_steps = 4
  l_tol = 1e-12
[]

[Outputs]
  [csv]
    type = CSV
    execute_on = TIMESTEP_END
  []
[]
