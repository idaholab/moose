ax=1
ay=1

[GlobalParams]
  u = ${ax}
  v = ${ay}
  pressure = 0
  tau_type = mod
  transient_term = true
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  xmax = 1
  ymax = 1
  elem_type = QUAD9
[]

[Variables]
  [./c]
    family = LAGRANGE
    order = SECOND
  [../]
[]

[Kernels]
  [./adv]
    type = Advection
    variable = c
    forcing_func = 'ffn'
  [../]
[]

[BCs]
  [./all]
    type = FunctionDirichletBC
    variable = c
    boundary = 'left right top bottom'
    function = 'c_func'
  [../]
[]

[Materials]
  [./mat]
    type = GenericConstantMaterial
    prop_names = 'mu rho'
    prop_values = '0 1'
  [../]
[]

[Functions]
  [./ffn]
    type = ParsedFunction
    expression = '${ax}*(0.14*pi*y*cos(0.2*pi*x*y) + 0.2*pi*cos(0.5*pi*x)) + ${ay}*(0.14*pi*x*cos(0.2*pi*x*y) + 0.4*pi*cos(pi*y))'
  [../]
  [./c_func]
    type = ParsedFunction
    expression = '0.4*sin(0.5*pi*x) + 0.4*sin(pi*y) + 0.7*sin(0.2*pi*x*y) + 0.5'
  [../]
  [./cx_func]
    type = ParsedFunction
    expression = '0.14*pi*y*cos(0.2*pi*x*y) + 0.2*pi*cos(0.5*pi*x)'
  [../]
[]

# [Executioner]
#   type = Steady
#   petsc_options_iname = '-pc_type -pc_factor_shift_type'
#   petsc_options_value = 'lu NONZERO'
# []

[Executioner]
  type = Transient
  num_steps = 10
  petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_view'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu NONZERO superlu_dist'
  line_search = 'none'
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-12
  nl_max_its = 10
  l_tol = 1e-6
  l_max_its = 10
  [./TimeStepper]
    dt = .05
    type = IterationAdaptiveDT
    cutback_factor = 0.4
    growth_factor = 1.2
    optimal_iterations = 20
  [../]
[]

[Outputs]
  [./exodus]
    type = Exodus
  [../]
  [./csv]
    type = CSV
  [../]
[]

[Postprocessors]
  [./L2c]
    type = ElementL2Error
    variable = c
    function = c_func
    outputs = 'console'    execute_on = 'timestep_end'
  [../]
  [./L2cx]
    type = ElementL2Error
    variable = cx
    function = cx_func
    outputs = 'console'    execute_on = 'timestep_end'
  [../]
[]

[AuxVariables]
  [./cx]
    family = MONOMIAL
    order = FIRST
  [../]
[]

[AuxKernels]
  [./cx_aux]
    type = VariableGradientComponent
    component = x
    variable = cx
    gradient_variable = c
  [../]
[]
