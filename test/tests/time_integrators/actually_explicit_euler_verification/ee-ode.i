# Tests that ActuallyExplicitEuler works with scalar variables.
#
# The ODE and IC used are the following:
#   du/dt = 2,       u(0) = 0
# Thus the solution is u(t) = 2*t.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Variables]
  [./u]
    family = SCALAR
    order = FIRST
    initial_condition = 0
  [../]
[]

[ScalarKernels]
  [./time]
    type = ODETimeDerivative
    variable = u
  [../]
  [./source]
    type = ParsedODEKernel
    variable = u
    expression = -2
  [../]
[]

[Executioner]
  type = Transient

  [./TimeIntegrator]
    type = ActuallyExplicitEuler
  [../]
  dt = 1
  num_steps = 5
[]

[Outputs]
  csv = true
[]
