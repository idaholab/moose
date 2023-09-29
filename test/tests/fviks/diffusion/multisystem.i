[Mesh]
  [gmg]
    type = CartesianMeshGenerator
    dim = 1
    ix = '50 50'
    dx = '1 1'
    subdomain_id = '0 1'
  []
  [sds]
    type = SideSetsBetweenSubdomainsGenerator
    input = gmg
    new_boundary = 'between'
    paired_block = '1'
    primary_block = '0'
  []
[]

[Problem]
  nl_sys_names = 'u v'
  error_on_jacobian_nonzero_reallocation = true
[]

[Variables]
  [u]
    type = MooseVariableFVReal
    nl_sys = 'u'
    block = 0
  []
  [v]
    type = MooseVariableFVReal
    nl_sys = 'v'
    block = 1
  []
[]

[FVKernels]
  [diff_u]
    type = FVDiffusion
    variable = u
    coeff = 3.0
  []
  [force_u]
    type = FVBodyForce
    variable = u
    function = 5
  []
  [diff_v]
    type = FVDiffusion
    variable = v
    coeff = 1.0
  []
  [force_v]
    type = FVBodyForce
    variable = v
    function = 5
  []
[]

[FVInterfaceKernels]
  [diff_ik]
    type = FVDiffusionInterface
    variable1 = u
    variable2 = v
    boundary = 'between'
    coeff1 = 3
    coeff2 = 1
    subdomain1 = 0
    subdomain2 = 1
  []
  [diff_ik_v]
    type = FVDiffusionInterface
    variable1 = v
    variable2 = u
    boundary = 'between'
    coeff1 = 1
    coeff2 = 3
    subdomain1 = 1
    subdomain2 = 0
  []
[]

[FVBCs]
  [left_u]
    type = FVDirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right_v]
    type = FVDirichletBC
    variable = v
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = SteadySolve2
  solve_type = 'NEWTON'
  petsc_options = '-snes_monitor'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  first_nl_sys_to_solve = 'u'
  second_nl_sys_to_solve = 'v'
  number_of_iterations = 200
  nl_abs_tol = 1e-10
[]

[Outputs]
  print_nonlinear_residuals = false
  print_linear_residuals = false
  exodus = true
[]
