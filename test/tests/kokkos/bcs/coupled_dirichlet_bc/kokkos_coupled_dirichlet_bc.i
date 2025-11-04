[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []

  [v]
    order = FIRST
    family = LAGRANGE
  []
[]

[KokkosKernels]
  [diff_u]
    type = KokkosDiffusion
    variable = u
  []

  [coupled_force_u]
    type = KokkosCoupledForce
    variable = u
    v = v
  []

  [diff_v]
    type = KokkosDiffusion
    variable = v
  []
[]

[KokkosBCs]
  # BCs on left
  # u: u=1
  # v: v=2
  [left_u]
    type = KokkosDirichletBC
    variable = u
    boundary = 3
    value = 1
  []

  [left_v]
    type = KokkosDirichletBC
    variable = v
    boundary = 3
    value = 2
  []

  # BCs on right
  # u: c*u + u^2 + v^2 = 9
  # v: no flux
  [right_u]
    type = KokkosCoupledDirichletBC
    variable = u
    boundary = 1
    value = 9
    v=v
  []
[]

[Preconditioning]
  [precond]
    type = SMP
    # 'full = true' is required for computeOffDiagJacobian() to get
    # called.  If you comment this out, you should see that this test
    # requires a different number of linear and nonlinear iterations.
    full = true
  []
[]

[Executioner]
  type = Steady

  # solve_type = 'PJFNK'
  solve_type = 'NEWTON'

  # Uncomment next line to disable line search.  With line search enabled, you must use full=true with Newton or else it will fail.
  # line_search = 'none'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'

  nl_rel_tol = 1e-10
  l_tol = 1e-12
  nl_max_its = 10
[]

[Outputs]
  exodus = true
[]
