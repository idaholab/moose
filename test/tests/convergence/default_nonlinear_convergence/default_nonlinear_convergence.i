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
    # With this, convergence should occur immediately (zero iterations)
    nl_abs_tol = 1e15
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
  verbose = true
[]

[Outputs]
  csv = true
  execute_on = 'FINAL'
[]
