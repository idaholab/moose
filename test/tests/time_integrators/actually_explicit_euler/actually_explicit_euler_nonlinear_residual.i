# Tests that the ActuallyExplicitEuler time integrator is able to compute the
# final nonlinear residual for a time step and have it retrieved by the
# Residual post-processor. The test checks that the post-processor has a value
# greater than 1e-15, since without monitoring, the residual value is reported
# as zero.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
  [../]
  [./v]
  [../]
[]

[Kernels]
  [./time_u]
    type = TimeDerivative
    variable = u
  [../]
  [./diff_u]
    type = CoefDiffusion
    variable = u
    coef = 0.1
    implicit = false
  [../]

  [./time_v]
    type = TimeDerivative
    variable = v
  [../]
  [./src_v]
    type = BodyForce
    variable = v
    function = 1000
    implicit = false
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 'left'
    value = 0
    implicit = false
  [../]
  [./right]
    type = MatchedValueBC
    variable = u
    boundary = 'right'
    v = v
    implicit = false
  [../]
[]

[Executioner]
  type = Transient

  dt = 0.001
  num_steps = 5

  l_tol = 1e-12

  [./TimeIntegrator]
    type = ActuallyExplicitEuler
    compute_final_nonlinear_residual = true
  [../]
[]

[Postprocessors]
  [./nonlinear_residual]
    type = Residual
  [../]

  [./residual_greater_than_zero]
    type = PostprocessorComparison
    value_a = nonlinear_residual
    comparison_type = greater_than
    value_b = 1e-15
    absolute_tolerance = 0
  [../]
[]

[Outputs]
  csv = true
  show = 'residual_greater_than_zero'
  execute_on = 'TIMESTEP_END'
[]
