# This is a test of the ForcingFunctionAux AuxKernel.
# The diffusion equation for u is solved with boundary conditions to force a gradient
# du/dx = 2, which is constant in time.
# du/dx is integrated over the unit square domain using a postprocessor, resulting in 2.
# The value of this postprocessor is supplied to the forcing function f used by
# the ForcingFunctionAux AuxKernel, which increments the AuxVariable T.
# Since the time step is 1, the value of T increases by 2 for each time step.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./grad_u_x]
    order = CONSTANT
    family = MONOMIAL
    initial_condition = 2
  [../]
  [./T]
    order = CONSTANT
    family = MONOMIAL
    initial_condition = 100
  [../]
[]

[Functions]
  [./u_ic_func]
    type = ParsedFunction
    expression = '2*x'
  [../]
  [./f]
    type = ParsedFunction
    symbol_names = f
    symbol_values = grad_int
    expression = f
  [../]
[]

[ICs]
  [./u_ic]
    type = FunctionIC
    variable = u
    function = u_ic_func
  [../]
[]

[Kernels]
  [./dudt]
    type = TimeDerivative
    variable = u
  [../]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[AuxKernels]
  [./grad_u_x_aux]
    type = VariableGradientComponent
    variable = grad_u_x
    component = x
    gradient_variable = u
  [../]
  [./T_increment]
    type = ForcingFunctionAux
    variable = T
    function = f
  [../]
[]

[Postprocessors]
  [./grad_int]
    type = ElementIntegralVariablePostprocessor
    variable = grad_u_x
    execute_on = 'INITIAL TIMESTEP_END'
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
    value = 2
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  nl_abs_tol = 1e-10
  num_steps = 2
  dt = 1
[]

[Outputs]
  exodus = true
[]

