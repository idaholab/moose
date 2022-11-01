[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx = 2
  ny = 2
  elem_type = QUAD9
[]

[Variables]
  [./u]
    family = LAGRANGE
    order = SECOND
  [../]
  [./lambda]
    family = SCALAR
    order = FIRST
  [../]
[]

[Kernels]
  [./diff]
    type = ADDiffusion
    variable = u
  [../]

  [./sk_lm]
    type = ADScalarLMKernel
    variable = u
    coupled_scalar = lambda
    kappa = lambda
    pp_name = pp
    value = 2.666666666666666
  [../]
[]

# With above block, not needed, and results in Petsc error messages
[Problem]
  kernel_coverage_check = false
  error_on_jacobian_nonzero_reallocation = true
[]

[BCs]
  [./bottom]
    type = ADDirichletBC
    variable = u
    boundary = 'bottom'
    value = 0
  [../]
  [./right]
    type = ADDirichletBC
    variable = u
    boundary = 'right'
    value = 0
  [../]
  [./top]
    type = ADDirichletBC
    variable = u
    boundary = 'top'
    value = 0
  [../]
  [./left]
    type = ADDirichletBC
    variable = u
    boundary = 'left'
    value = 0
  [../]
[]

[Postprocessors]
  # integrate the volume of domain since original objects set 
  # int(phi)=V0, rather than int(phi-V0)=0
  [./pp]
    type = FunctionElementIntegral
    function = 1
    execute_on = initial
  [../]
[]

# Force LU decomposition, nonlinear iterations, to check Jacobian terms with single factorization
[Executioner]
  type = Steady
  residual_and_jacobian_together = true
  nl_rel_tol = 1e-9
  l_tol = 1.e-10
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu superlu_dist'
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
  hide = lambda
[]
