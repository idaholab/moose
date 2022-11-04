#
# Testing a solution that is second order in space and first order in time
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

[AuxVariables]
  [./a]
    family = SCALAR
    order = FIRST
  [../]
  [./b]
    family = SCALAR
    order = FIRST
  [../]
  [./c]
    family = SCALAR
    order = FIRST
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[ICs]
  [./a_ic]
    type = ScalarConstantIC
    variable = a
    value = 0
  [../]
  [./b_ic]
    type = ScalarConstantIC
    variable = b
    value = 2
  [../]
[]

[Functions]
  [./exact_fn]
    type = ParsedFunction
    expression = t
  [../]

  [./a_fn]
    type = ParsedFunction
    expression = t
  [../]
  [./b_fn]
    type = ParsedFunction
    expression = (4-t)/2
  [../]
[]

# NOTE: The execute_on = 'timestep_end' is crucial for this test. Without it
# the aux values would be updated during the residual formation and we would
# end up with the right value at the end of the time step. With this flag on,
# the dependencies has to be correct for this test to work. Otherwise the
# values of 'c' will be lagged.

[AuxScalarKernels]
  [./c_saux]
    type = QuotientScalarAux
    variable = c
    numerator = a
    denominator = b
    execute_on = 'timestep_end'
  [../]
  [./a_saux]
    type = FunctionScalarAux
    variable = a
    function = a_fn
    execute_on = 'timestep_end'
  [../]
  [./b_saux]
    type = FunctionScalarAux
    variable = b
    function = b_fn
    execute_on = 'timestep_end'
  [../]
[]

[Kernels]
  [./ie]
    type = TimeDerivative
    variable = u
  [../]

  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = exact_fn
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'implicit-euler'

  solve_type = 'PJFNK'
  start_time = 0.0
  num_steps = 2
  dt = 1
[]

[Outputs]
  exodus = true
[]
