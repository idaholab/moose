#
# Example 18 modified to use parsed ODE kernels.
#
# The ParsedODEKernel takes expression expressions in the input file and computes
# Jacobian entries via automatic differentiation. It allows for rapid development
# of new models without the need for code recompilation.
#
# This input file should produce the exact same result as ex18.i
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 10
  ny = 10
  elem_type = QUAD4
[]

[Functions]
  # ODEs
  [./exact_x_fn]
    type = ParsedFunction
    expression = (-1/3)*exp(-t)+(4/3)*exp(5*t)
  [../]
  [./exact_y_fn]
    type = ParsedFunction
    expression = (2/3)*exp(-t)+(4/3)*exp(5*t)
  [../]
[]

[Variables]
  [./diffused]
    order = FIRST
    family = LAGRANGE
  [../]

  # ODE variables
  [./x]
    family = SCALAR
    order = FIRST
    initial_condition = 1
  [../]
  [./y]
    family = SCALAR
    order = FIRST
    initial_condition = 2
  [../]

[]

[Kernels]
  [./td]
    type = TimeDerivative
    variable = diffused
  [../]
  [./diff]
    type = Diffusion
    variable = diffused
  [../]
[]

[ScalarKernels]
  [./td1]
    type = ODETimeDerivative
    variable = x
  [../]

  #
  # This parsed expression ODE Kernel behaves exactly as the ImplicitODEx kernel
  # in the main example. Checkout ImplicitODEx::computeQpResidual() in the
  # source code file ImplicitODEx.C to see the matching residual function.
  #
  # The ParsedODEKernel automaticaly generates the On- and Off-Diagonal Jacobian
  # entries.
  #
  [./ode1]
    type = ParsedODEKernel
    expression = '-3*x - 2*y'
    variable = x
    coupled_variables = y
  [../]

  [./td2]
    type = ODETimeDerivative
    variable = y
  [../]

  #
  # This parsed expression ODE Kernel behaves exactly as the ImplicitODEy Kernel
  # in the main example.
  #
  [./ode2]
    type = ParsedODEKernel
    expression = '-4*x - y'
    variable = y
    coupled_variables = x
  [../]
[]


[BCs]
  [./right]
    type = ScalarDirichletBC
    variable = diffused
    boundary = 1
    scalar_var = x
  [../]

  [./left]
    type = ScalarDirichletBC
    variable = diffused
    boundary = 3
    scalar_var = y
  [../]
[]

[Postprocessors]
 # to print the values of x, y into a file so we can plot it
  [./x_pp]
    type = ScalarVariable
    variable = x
    execute_on = timestep_end
  [../]

  [./y_pp]
    type = ScalarVariable
    variable = y
    execute_on = timestep_end
  [../]

  [./exact_x]
    type = FunctionValuePostprocessor
    function = exact_x_fn
    execute_on = timestep_end
  [../]

  [./exact_y]
    type = FunctionValuePostprocessor
    function = exact_y_fn
    execute_on = timestep_end
    point = '0 0 0'
  [../]

  # Measure the error in ODE solution for 'x'.
  [./l2err_x]
    type = ScalarL2Error
    variable = x
    function = exact_x_fn
  [../]

  # Measure the error in ODE solution for 'y'.
  [./l2err_y]
    type = ScalarL2Error
    variable = y
    function = exact_y_fn
  [../]
[]


[Executioner]
  type = Transient
  start_time = 0
  dt = 0.01
  num_steps = 10
  solve_type = 'PJFNK'
[]

[Outputs]
  file_base = 'ex18_out'
  exodus = true
[]
