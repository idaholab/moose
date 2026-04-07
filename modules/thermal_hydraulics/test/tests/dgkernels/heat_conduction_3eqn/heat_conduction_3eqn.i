# MMS test for HeatConduction3EqnDGKernel.
# Run mms_derivation.py to print out MMS source.

k = 10.0
A = 0.1
C = 100

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmin = 0
  xmax = 1
[]

[Variables]
  [T]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 0
  []
[]

[AuxVariables]
  [A]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = ${A}
  []
[]

[Materials]
  [T_mat]
    type = ADCoupledVariableValueMaterial
    coupled_variable = T
    prop_name = T
  []
  [k_mat]
    type = ADGenericConstantMaterial
    prop_names = 'k'
    prop_values = '${k}'
  []
  [dir_mat]
    type = DirectionMaterial
  []
[]

[DGKernels]
  [heat_cond]
    type = HeatConduction3EqnDGKernel
    variable = T
    direction = direction
    T = T
    k = k
    A = A
  []
[]

[Kernels]
  [time]
    type = TimeDerivative
    variable = T
  []
  [mms_source]
    type = BodyForce
    variable = T
    function = mms_source_fn
  []
[]

[Functions]
  [T_exact_fn]
    type = ParsedFunction
    expression = 'C*t*cos(2*pi*x)'
    symbol_names = 'C'
    symbol_values = '${C}'
  []
  [mms_source_fn]
    type = ParsedFunction
    expression = '4*pi^2*A*C*k*t*cos(2*pi*x) + C*cos(2*pi*x)'
    symbol_names = 'A C k'
    symbol_values = '${A} ${C} ${k}'
  []
[]

[Executioner]
  type = Transient
  scheme = implicit-euler
  dt = 1
  num_steps = 1
  abort_on_solve_fail = true

  solve_type = PJFNK
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-8
  nl_max_its = 30
  l_tol = 1e-3
  l_max_its = 10
[]

[Postprocessors]
  [error]
    type = ElementL2Error
    variable = T
    function = T_exact_fn
  []
  [h]
    type = AverageElementSize
  []
[]

[Outputs]
  csv = true
[]
