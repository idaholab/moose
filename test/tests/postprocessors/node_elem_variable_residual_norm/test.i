# Test ODEs:
#   blockA: du/dt = 3 -> initial residual = 3
#   blockB: du/dt = 4 -> initial residual = 4

[Mesh]
  # A 1D (or 2D or 3D) block is needed to prevent an error
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 1
    subdomain_ids = 0
    subdomain_name = dummy_block
  []
  [mgA]
    type = ElementGenerator
    elem_type = NODEELEM
    nodal_positions = '1 2 3'
    element_connectivity = '0'
    subdomain_id = 1
    subdomain_name = blockA
  []
  [mgB]
    type = ElementGenerator
    elem_type = NODEELEM
    nodal_positions = '4 5 6'
    element_connectivity = '0'
    subdomain_id = 2
    subdomain_name = blockB
  []
  [combiner]
    type = CombinerGenerator
    inputs = 'gen mgA mgB'
  []
[]

[Variables]
  [u]
    block = 'blockA blockB'
    initial_condition = 0
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Kernels]
  [time_derivative_blockA]
    type = TimeDerivative
    variable = u
    block = blockA
  []
  [body_force_blockA]
    type = BodyForce
    variable = u
    block = blockA
    function = 3
  []
  [time_derivative_blockB]
    type = TimeDerivative
    variable = u
    block = blockB
  []
  [body_force_blockB]
    type = BodyForce
    variable = u
    block = blockB
    function = 4
  []
[]

[Postprocessors]
  [res_blockA]
    type = NodeElemVariableResidualNorm
    variable = u
    block = blockA
    execute_on = 'FINAL'
  []
  [res_blockB]
    type = NodeElemVariableResidualNorm
    variable = u
    block = blockB
    execute_on = 'FINAL'
  []
[]

[Convergence]
  [nl_conv]
    type = IterationCountConvergence
    max_iterations = 1
    converge_at_max_iterations = true
  []
[]

[Problem]
  kernel_coverage_check = false
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 1
  solve_type = NEWTON
  # line_search = none

  # The Postprocessor does not work on INITIAL since the nonlinear residual
  # vector has not been updated at that point, so we prevent the solution from
  # changing using a 0 relaxation factor.

  petsc_options_iname = '-snes_linesearch_damping'
  petsc_options_value = '0'

  nonlinear_convergence = nl_conv
[]

[Outputs]
  csv = true
  execute_on = 'FINAL'
[]
