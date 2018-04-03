# This example demonstrates ability to set Dirichlet boundary conditions for LAGRANGE_VEC variables

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
    family = LAGRANGE_VEC
    order = FIRST
  [../]
  [./v]
    family = LAGRANGE_VEC
    order = FIRST
  [../]
  [./w]
    family = LAGRANGE_VEC
    order = FIRST
  [../]
  [./s]
    family = LAGRANGE_VEC
    order = FIRST
  [../]
[]

[Kernels]
  [./u_diff]
    type = VectorDiffusion
    variable = u
  [../]
  [./v_diff]
    type = VectorDiffusion
    variable = v
  [../]
  [./w_diff]
    type = VectorDiffusion
    variable = w
  [../]
  [./s_diff]
    type = VectorDiffusion
    variable = s
  [../]
  [./v_coupled_diff]
    type = CoupledVectorDiffusion
    variable = v
    v = u
  [../]
  [./w_coupled_diff]
    type = CoupledVectorDiffusion
    variable = w
    v = u
    state = old
  [../]
  [./s_coupled_diff]
    type = CoupledVectorDiffusion
    variable = s
    v = u
    state = older
  [../]
[]

[BCs]
  [./left_u]
    type = LagrangeVecDirichletBC
    variable = u
    values = '0 0 0'
    boundary = 'left'
  [../]
  [./left_v]
    type = LagrangeVecDirichletBC
    variable = v
    values = '0 0 0'
    boundary = 'left'
  [../]
  [./left_w]
    type = LagrangeVecDirichletBC
    variable = w
    values = '0 0 0'
    boundary = 'left'
  [../]
  [./left_s]
    type = LagrangeVecDirichletBC
    variable = s
    values = '0 0 0'
    boundary = 'left'
  [../]
  [./right_u]
    type = LagrangeVecFunctionDirichletBC
    variable = u
    boundary = 'right'
    x_exact_soln = 'x_exact'
    y_exact_soln = 'y_exact'
  [../]
  [./right_v]
    type = LagrangeVecFunctionDirichletBC
    variable = v
    boundary = 'right'
    x_exact_soln = 'x_exact'
    y_exact_soln = 'y_exact'
  [../]
  [./right_w]
    type = LagrangeVecFunctionDirichletBC
    variable = w
    boundary = 'right'
    x_exact_soln = 'x_exact_old'
    y_exact_soln = 'y_exact_old'
  [../]
  [./right_s]
    type = LagrangeVecFunctionDirichletBC
    variable = s
    boundary = 'right'
    x_exact_soln = 'x_exact_older'
    y_exact_soln = 'y_exact_older'
  [../]
[]

[Functions]
  [./x_exact]
    type = ParsedFunction
    value = 't'
  [../]
  [./y_exact]
    type = ParsedFunction
    value = 't'
  [../]
  [./x_exact_old]
    type = ParsedFunction
    value = 'if(t < 1, 0, t - 1)'
  [../]
  [./y_exact_old]
    type = ParsedFunction
    value = 'if(t < 1, 0, t - 1)'
  [../]
  [./x_exact_older]
    type = ParsedFunction
    value = 'if(t < 2, 0, t - 2)'
  [../]
  [./y_exact_older]
    type = ParsedFunction
    value = 'if(t < 2, 0, t - 2)'
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 3
  solve_type = 'NEWTON'
  petsc_options = '-ksp_converged_reason -snes_converged_reason'
  nl_max_its = 3
  l_max_its = 20
  dtmin = 1
[]

[Outputs]
  exodus = true
[]
