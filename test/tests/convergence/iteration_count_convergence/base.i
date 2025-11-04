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

[Postprocessors]
  [nonlin_its]
    type = NumNonlinearIterations
    execute_on = 'TIMESTEP_END'
  []
[]

[Convergence]
  [test_conv]
    type = SuppliedStatusConvergence
    min_iterations = 2
    max_iterations = 4
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  line_search = none
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'hypre'
  nonlinear_convergence = test_conv
[]

[Outputs]
  csv = true
  execute_on = 'FINAL'
[]
