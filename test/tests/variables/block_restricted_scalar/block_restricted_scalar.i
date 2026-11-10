[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
  []
  [right_half]
    type = SubdomainBoundingBoxGenerator
    input = gen
    block_id = 1
    bottom_left = '0.5 0 0'
    top_right = '1 1 0'
  []
[]

[Variables]
  [u]
  []
  [lambda]
    family = SCALAR
    block = 1
  []
  [lambda2]
    family = SCALAR
    block = 1
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [lm]
    type = ScalarLagrangeMultiplier
    variable = u
    lambda = lambda
  []
[]

[ScalarKernels]
  [constraint]
    type = AverageValueConstraint
    variable = lambda
    pp_name = average
    value = 0.5
  []
  [coupled]
    type = ParsedODEKernel
    variable = lambda
    expression = 'lambda2'
    coupled_variables = lambda2
  []
  [null]
    type = NullScalarKernel
    variable = lambda2
    jacobian_fill = 1
  []
[]

[Postprocessors]
  [average]
    type = ElementIntegralVariablePostprocessor
    variable = u
    execute_on = linear
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  # The Lagrange multiplier makes the system indefinite, so use a direct solve
  # on this small problem.
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]
