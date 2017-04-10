[Mesh]
 type = GeneratedMesh
 dim = 2
 xmin = 0
 xmax = 10
 ymin = 0
 ymax = 10
 elem_type = QUAD4
 nx = 64
 ny = 64

 uniform_refine = 0
[]

# the minimum eigenvalue of this problem is 2*(PI/a)^2;
# Its inverse is 0.5*(a/PI)^2 = 5.0660591821169. Here a is equal to 10.

[Variables]
  active = 'u'
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./rhs]
    type = Reaction
    variable = u
    eigen_kernel = true
  [../]
[]

[BCs]
  [./homogeneous]
    type = DirichletBC
    variable = u
    boundary = '0 1 2 3'
    value = 0
    use_displaced_mesh = true
  [../]
[]

[Problem]
  type = EigenProblem
  eigen_problem_type = gen_non_hermitian
  n_eigen_pairs = 1
  n_basis_vectors = 18
[]

[Executioner]
  type = Steady
  eigen_solve_type = MONOLITH_NEWTON
  petsc_options_iname = '-snes_mf_operator'
  petsc_options_value = '0'
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
  [./console]
    type = Console
    outlier_variable_norms = false
  [../]
[]
