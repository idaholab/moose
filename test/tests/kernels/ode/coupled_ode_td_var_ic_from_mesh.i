[Mesh]
  type = FileMesh
  file = 'coupled_ode_td_out.e'
[]

[Variables]
  [./f]
    family = SCALAR
    order = FIRST
    initial_from_file_var = f
    initial_from_file_timestep = 'LATEST'
  [../]
  [./f_times_mult]
    family = SCALAR
    order = FIRST
    initial_from_file_var = f_times_mult
    initial_from_file_timestep = 'LATEST'
  [../]
[]

[ScalarKernels]
  [./dT]
    type = CoupledODETimeDerivative
    variable = f
    v = f_times_mult
  [../]

  [./src]
    type = ParsedODEKernel
    variable = f
    function = '-1'
  [../]

  [./f_times_mult_1]
    type = ParsedODEKernel
    variable = f_times_mult
    function = 'f_times_mult'
  [../]

  [./f_times_mult_2]
    type = ParsedODEKernel
    variable = f_times_mult
    function = '-f * g'
    args = 'f g'
  [../]
[]

[AuxVariables]
  [./g]
    family = SCALAR
    order = FIRST
  [../]
[]

[Functions]
  [./function_g]
    type = ParsedFunction
    value = '(1 + t)'
  [../]
[]

[AuxScalarKernels]
  [./set_g]
    type = FunctionScalarAux
    function = function_g
    variable = g
    execute_on = 'linear initial'
  [../]
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 3
  nl_abs_tol = 1e-9
[]

[Outputs]
  csv = true
[]
