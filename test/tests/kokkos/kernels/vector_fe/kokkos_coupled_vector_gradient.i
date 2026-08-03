# This example demonstrates ability to set Dirichlet boundary conditions for LAGRANGE_VEC variables

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [u]
    family = LAGRANGE_VEC
    order = FIRST
  []
  [v]
    family = LAGRANGE_VEC
    order = FIRST
  []
  [w]
    family = LAGRANGE_VEC
    order = FIRST
  []
  [s]
    family = LAGRANGE_VEC
    order = FIRST
  []
  [q]
  []
[]

[Kernels]
  [u_diff]
    type = KokkosVectorDiffusion
    variable = u
  []
  [v_diff]
    type = KokkosVectorDiffusion
    variable = v
  []
  [w_diff]
    type = KokkosVectorDiffusion
    variable = w
  []
  [s_diff]
    type = KokkosVectorDiffusion
    variable = s
  []
  [v_coupled_diff]
    type = KokkosCoupledVectorDiffusion
    variable = v
    v = u
  []
  [w_coupled_diff]
    type = KokkosCoupledVectorDiffusion
    variable = w
    v = u
    state = old
  []
  [s_coupled_diff]
    type = KokkosCoupledVectorDiffusion
    variable = s
    v = u
    state = older
  []
  [q_diff]
    type = KokkosDiffusion
    variable = q
  []
[]

[BCs]
  [left_u]
    type = KokkosVectorDirichletBC
    variable = u
    values = '0 0 0'
    boundary = 'left'
  []
  [left_v]
    type = KokkosVectorDirichletBC
    variable = v
    values = '0 0 0'
    boundary = 'left'
  []
  [left_w]
    type = KokkosVectorDirichletBC
    variable = w
    values = '0 0 0'
    boundary = 'left'
  []
  [left_s]
    type = KokkosVectorDirichletBC
    variable = s
    values = '0 0 0'
    boundary = 'left'
  []
  [right_u]
    type = KokkosVectorFunctionDirichletBC
    variable = u
    boundary = 'right'
    function_x = 'x_exact'
    function_y = 'y_exact'
  []
  [right_v]
    type = KokkosVectorFunctionDirichletBC
    variable = v
    boundary = 'right'
    function_x = 'x_exact'
    function_y = 'y_exact'
  []
  [right_w]
    type = KokkosVectorFunctionDirichletBC
    variable = w
    boundary = 'right'
    function_x = 'x_exact_old'
    function_y = 'y_exact_old'
  []
  [right_s]
    type = KokkosVectorFunctionDirichletBC
    variable = s
    boundary = 'right'
    function_x = 'x_exact_older'
    function_y = 'y_exact_older'
  []
  [left_q]
    type = KokkosDirichletBC
    variable = q
    boundary = 'left'
    value = 1
  []
  [right_q]
    type = KokkosNeumannBC
    variable = q
    boundary = 'right'
    value = 1
  []
[]

[Functions]
  [x_exact]
    type = KokkosParsedFunction
    expression = 't'
  []
  [y_exact]
    type = KokkosParsedFunction
    expression = 't'
  []
  [x_exact_old]
    type = KokkosParsedFunction
    expression = 'if(t < 1, 0, t - 1)'
  []
  [y_exact_old]
    type = KokkosParsedFunction
    expression = 'if(t < 1, 0, t - 1)'
  []
  [x_exact_older]
    type = KokkosParsedFunction
    expression = 'if(t < 2, 0, t - 2)'
  []
  [y_exact_older]
    type = KokkosParsedFunction
    expression = 'if(t < 2, 0, t - 2)'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 3
  solve_type = 'NEWTON'
  petsc_options = '-ksp_converged_reason -snes_converged_reason'
  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '100'
  nl_max_its = 3
  l_max_its = 100
  dtmin = 1
[]

[Outputs]
  exodus = true
[]
