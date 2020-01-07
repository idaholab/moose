[Mesh]
  block_id = '0 1'
  block_name = 'block_zero block_one'
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
  [./subdomain_id]
    input = gen
    type = SubdomainIDGenerator
    subdomain_id = 1
  [../]
[]

[Variables]
  [./u]
    family = LAGRANGE
    order = FIRST
  [../]
  [./v]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./w]
    family = SCALAR
    order = FIRST
  [../]
[]

[Kernels]
  [./u_kernel]
    type = Diffusion
    variable = u
  [../]
  [./v_kernel]
    type = Diffusion
    variable = v
  [../]
[]

[BCs]
  [./u_bc]
    type = DirichletBC
    variable = u
    value = 100
    boundary = left
  [../]
  [./v_bc]
    type = NeumannBC
    variable = v
    value = 100
    boundary = left
  [../]
[]

[ScalarKernels]
  [./w_kernel]
    type = AlphaCED
    variable = w
    value = 100
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'

  # For this test, we don't actually want the solution to converge because we
  # want nonzero nonlinear residual entries at the end of the time step.
  nl_abs_tol = 0.999
  nl_rel_tol = 0.999
  l_max_its = 1
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'none'
[]

[Debug]
  show_top_residuals = 10
[]
