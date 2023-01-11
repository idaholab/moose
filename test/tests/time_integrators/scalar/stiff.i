# This is a linear model problem described in Frank et al, "Order
# results for implicit Runge-Kutta methods applied to stiff systems",
# SIAM J. Numerical Analysis, vol. 22, no. 3, 1985, pp. 515-534.
#
# Problems "PL" and "PNL" from page 527 of the paper:
# { dy1/dt = lambda*y1 + y2**p, y1(0) = -1/(lambda+p)
# { dy2/dt = -y2,               y2(0) = 1
#
# The exact solution is:
# y1 = -exp(-p*t)/(lambda+p)
# y2 = exp(-t)
#
# According to the following paragraph from the reference above, the
# p=1 version of this problem should not exhibit order reductions
# regardless of stiffness, while the nonlinear version (p>=2) will
# exhibit order reductions down to the "stage order" of the method for
# lambda large, negative.

# Use Dollar Bracket Expressions (DBEs) to set the value of LAMBDA in
# a single place.  You can also set this on the command line with
# e.g. LAMBDA=-4, but note that this does not seem to override the
# value set in the input file.  This is a bit different from the way
# that command line values normally work...
# Note that LAMBDA == Y2_EXPONENT is not allowed!
# LAMBDA = -10
# Y2_EXPONENT = 2

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 1
  ny = 1
  elem_type = QUAD4
[]

[Variables]
  [./y1]
    family = SCALAR
    order = FIRST
  [../]
  [./y2]
    family = SCALAR
    order = FIRST
  [../]
[]

[ICs]
  [./y1_init]
    type = FunctionScalarIC
    variable = y1
    function = y1_exact
  [../]
  [./y2_init]
    type = FunctionScalarIC
    variable = y2
    function = y2_exact
  [../]
[]

[ScalarKernels]
  [./y1_time]
    type = ODETimeDerivative
    variable = y1
  [../]
  [./y1_space]
    type = ParsedODEKernel
    variable = y1
    expression = '-(${LAMBDA})*y1 - y2^${Y2_EXPONENT}'
    coupled_variables = 'y2'
  [../]
  [./y2_time]
    type = ODETimeDerivative
    variable = y2
  [../]
  [./y2_space]
    type = ParsedODEKernel
    variable = y2
    expression = 'y2'
  [../]
[]

[Executioner]
  type = Transient
  [./TimeIntegrator]
    type = LStableDirk2
  [../]
  start_time = 0
  end_time = 1
  dt = 0.125
  solve_type = 'PJFNK'
  nl_max_its = 6
  nl_abs_tol = 1.e-13
  nl_rel_tol = 1.e-32 # Force nl_abs_tol to be used.
  line_search = 'none'
[]

[Functions]
  [./y1_exact]
    type = ParsedFunction
    expression = '-exp(-${Y2_EXPONENT}*t)/(lambda+${Y2_EXPONENT})'
    symbol_names = 'lambda'
    symbol_values = ${LAMBDA}
  [../]
  [./y2_exact]
    type = ParsedFunction
    expression = exp(-t)
  [../]
[]

[Postprocessors]
  [./error_y1]
    type = ScalarL2Error
    variable = y1
    function = y1_exact
    execute_on = 'initial timestep_end'
  [../]
  [./error_y2]
    type = ScalarL2Error
    variable = y2
    function = y2_exact
    execute_on = 'initial timestep_end'
  [../]
  [./max_error_y1]
    # Estimate ||e_1||_{\infty}
    type = TimeExtremeValue
    value_type = max
    postprocessor = error_y1
    execute_on = 'initial timestep_end'
  [../]
  [./max_error_y2]
    # Estimate ||e_2||_{\infty}
    type = TimeExtremeValue
    value_type = max
    postprocessor = error_y2
    execute_on = 'initial timestep_end'
  [../]
  [./value_y1]
    type = ScalarVariable
    variable = y1
    execute_on = 'initial timestep_end'
  [../]
  [./value_y2]
    type = ScalarVariable
    variable = y2
    execute_on = 'initial timestep_end'
  [../]
  [./value_y1_abs_max]
    type = TimeExtremeValue
    value_type = abs_max
    postprocessor = value_y1
    execute_on = 'initial timestep_end'
  [../]
  [./value_y2_abs_max]
    type = TimeExtremeValue
    value_type = abs_max
    postprocessor = value_y2
    execute_on = 'initial timestep_end'
  [../]
[]

[Outputs]
  csv = true
[]
