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
  [rhs]
    type = MassEigenKernel
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

[Executioner]
  type = NonlinearEigen

  bx_norm = 'unorm'

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Postprocessors]
  [unorm]
    type = ElementIntegralVariablePostprocessor
    variable = u
    # execute on residual is important for nonlinear eigen solver!
    execute_on = linear
  []
[]

[Outputs]
  exodus = true
  perf_graph = true
[]

[MultiApps]
  [full_solve]
    type = FullSolveMultiApp
    # not setting app_type to use the same app type of parent, i.e. MooseTestApp
    execute_on = initial
    positions = '0 0 0'
    input_files = sub.i
  []
[]
