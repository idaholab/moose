# This tests if the correct number of nonlinear and linear iterations for a time
# step are recovered for each time integrator scheme.
#
# The gold files for each time integrator scheme were created manually by
# observing the numbers of iterations per time step.

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

[Kernels]
  [./time_der]
    type = TimeDerivative
    variable = u
  [../]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    preset = false
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    preset = false
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Transient
  [./TimeIntegrator]
    # The time integrator type is provided by the tests file
  [../]
  num_steps = 2
  abort_on_solve_fail = true
  dt = 1e-4

  nl_abs_tol = 1e-8
  nl_rel_tol = 0
  nl_max_its = 5
[]

[Postprocessors]
  [./num_nonlinear_iterations]
    type = NumNonlinearIterations
  [../]
  [./num_linear_iterations]
    type = NumLinearIterations
  [../]
[]

[Outputs]
  csv = true
[]
