# Solves the predator-prey ODEs
# x = hares, y = foxes
# dx/dt = a * x - b * x * y
# dy/dt = -c * y + d * x * y
a = 1.1 # 0.68537768 #
b = 0.4 # 0.10066489 #
c = 0.4 # 0.76217276 #
d = 0.1 # 0.434863 #

end_time = 1.5


[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  # x is the number of hares
  [x]
    family = SCALAR
    order = FIRST
    initial_condition = 10
  []
  # y is the number of foxes
  [y]
    family = SCALAR
    order = FIRST
    initial_condition = 10
  []
[]

[Functions]
  [log_of_x]
    type = ParsedFunction
    expression = 'log(a)' #
    symbol_names = 'a'
    symbol_values = 'x1'
  []
  [log_of_y]
    type = ParsedFunction
    expression = 'log(a)' #
    symbol_names = 'a'
    symbol_values = 'y1'
  []
[]

[ScalarKernels]
  [tdx]
    type = ODETimeDerivative
    variable = x
  []
  [rhsx]
    type = ParsedODEKernel
    expression = '-(${a} * x - ${b} * x * y)'
    variable = x
    coupled_variables = y # args
  []

  [tdy]
    type = ODETimeDerivative
    variable = y
  []
  [rhsy]
    type = ParsedODEKernel
    expression = '-(-${c} * y + ${d} * x * y)'
    variable = y
    coupled_variables = x # args
  []
[]

[Executioner]
  type = Transient
  start_time = 0
  [TimeIntegrator]
    type = LStableDirk4
  []
  dt = 0.05
  end_time = ${end_time}
  solve_type = 'PJFNK'
[]

[Postprocessors]
  [x1]
    type = ScalarVariable
    variable = x
    execute_on = timestep_end
  []
  [y1]
    type = ScalarVariable
    variable = y
    execute_on = timestep_end
  []
  [log_x]
    type = FunctionValuePostprocessor
    function = 'log_of_x'
    execute_on = timestep_end
  []
  [log_y]
    type = FunctionValuePostprocessor
    function = 'log_of_y'
    execute_on = timestep_end
  []
[]

[Outputs]
  csv = false
  console = false
[]
