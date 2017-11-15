[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 10
  ymin = 0
  ymax = 10
  elem_type = QUAD4
  nx = 8
  ny = 8
[]

# the minimum eigenvalue of this problem is 2*(PI/a)^2;
# Its inverse is 0.5*(a/PI)^2 = 5.0660591821169. Here a is equal to 10.

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
    vector_tags = 'nontime Ax_tag'
  [../]

  [./rhs]
    type = Reaction
    variable = u
    eigen_kernel = true
    vector_tags = 'nontime Bx_tag'
  [../]
[]

[BCs]
  [./homogeneous]
    type = DirichletBC
    variable = u
    boundary = '0 1 2 3'
    value = 0
    vector_tags = 'nontime Ax_tag'
  [../]
  [./eigen]
    type = DirichletBC
    variable = u
    boundary = '0 1 2 3'
    value = 0
    zero_residual = true
    vector_tags = 'Bx_tag'
  [../]
[]

[Executioner]
  type = Eigenvalue
  solve_type = MF_MONOLITH_NEWTON
  eigen_problem_type = GEN_NON_HERMITIAN
[]

[VectorPostprocessors]
  [./eigenvalues]
    type = Eigenvalues
    execute_on = 'timestep_end'
  [../]
[]

[Outputs]
  csv = true
  file_base = monolith_newton
  execute_on = 'timestep_end'
[]
