[Mesh]
  file = 'gold/ne_ic_out.e'
[]

# the minimum eigenvalue of this problem is 2*(PI/a)^2;
# Its inverse is 0.5*(a/PI)^2 = 5.0660591821169. Here a is equal to 10.

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
    # Use a good initial so that Newton can converge when we do not use free power iterations
    initial_from_file_var = u
    initial_from_file_timestep = 1
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./rhs]
    type = CoefReaction
    variable = u
    coefficient = -1.0
    extra_vector_tags = 'eigen'
  [../]
[]

[BCs]
  [./homogeneous]
    type = DirichletBC
    variable = u
    boundary = '0 1 2 3'
    value = 0
  [../]
  [./eigen]
    type = EigenDirichletBC
    variable = u
    boundary = '0 1 2 3'
  [../]
[]

[Executioner]
  type = Eigenvalue
  solve_type = PJFNK
  free_power_iterations = 0
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-6
[]

[VectorPostprocessors]
  [./eigenvalues]
    type = Eigenvalues
    execute_on = 'timestep_end'
  [../]
[]

[Outputs]
  exodus = true
  csv = true
  execute_on = 'timestep_end'
[]
