# Part 1: run a transient and write a checkpoint holding the solution for a nodal (LAGRANGE),
# a high-order elemental (MONOMIAL, order FIRST) and a scalar variable. Part 2 seeds its initial
# conditions from this checkpoint via 'initial_from_file_var'.

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
  []
[]

[Variables]
  [u]
    family = LAGRANGE
    order = FIRST
  []
  [ue]
    family = MONOMIAL
    order = FIRST
  []
  [lambda]
    family = SCALAR
    order = FIRST
  []
[]

[Kernels]
  [u_time]
    type = TimeDerivative
    variable = u
  []
  [u_diff]
    type = Diffusion
    variable = u
  []
  [ue_time]
    type = TimeDerivative
    variable = ue
  []
  [ue_reaction]
    type = Reaction
    variable = ue
  []
  [ue_source]
    type = BodyForce
    variable = ue
    function = 'x + 2*y + 3*t'
  []
[]

[ScalarKernels]
  [lambda_time]
    type = ODETimeDerivative
    variable = lambda
  []
  [lambda_source]
    type = ParsedODEKernel
    variable = lambda
    expression = '-1'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Postprocessors]
  [u_integral]
    type = ElementIntegralVariablePostprocessor
    variable = u
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [ue_integral]
    type = ElementIntegralVariablePostprocessor
    variable = ue
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [lambda_value]
    type = ScalarVariable
    variable = lambda
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  num_steps = 3
  dt = 0.5
  nl_abs_tol = 1e-10
[]

[Outputs]
  exodus = true
  csv = true
  checkpoint = true
[]
