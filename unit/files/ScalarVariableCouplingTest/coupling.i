[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
[]

[Variables]
  [u][]
  [lambda]
    family = SCALAR
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
    # Deliberately request only the diagonal blocks: no off_diag_row/off_diag_column for u/lambda,
    # and no full=true.
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  # The Lagrange multiplier's own diagonal Jacobian block is exactly zero (a saddle-point system),
  # so a real factorization needs a nonzero pivot shift to avoid failing on that block, independent
  # of whatever coupling this test requests.
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
[]

[Problem]
  # Fail loudly rather than silently reallocating if any block ever gets inserted outside the
  # sparsity that was actually requested.
  error_on_jacobian_nonzero_reallocation = true
[]
