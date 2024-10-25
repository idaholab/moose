[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
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

[Convergence]
  [res_conv]
    type = DefaultNonlinearConvergence
    # With default tolerances, the solve takes 2 iterations, but with this
    # value, the solve takes only 1:
    nl_abs_tol = 1e-5
  []
[]

[Postprocessors]
  [num_nl_its]
    type = NumNonlinearIterations
    execute_on = 'TIMESTEP_END'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'hypre'
  nonlinear_convergence = res_conv
[]

[Outputs]
  csv = true
  execute_on = 'FINAL'
[]
