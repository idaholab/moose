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

 uniform_refine = 0
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
  [./v]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff_u]
    type = Diffusion
    variable = u
  [../]
  [./diff_v]
    type = Diffusion
    variable = v
  [../]

  [./rhs]
    type = CoupledForce
    variable = u
    v = v
    eigen_kernel = true
    cof = -1.0
  [../]
  [./src_v]
    type = CoupledForce
    variable = v
    v = u
    eigen_kernel = true
    cof = -1.0
  [../]
[]

[BCs]
  [./homogeneous_u]
    type = DirichletBC
    variable = u
    boundary = '0 1 2 3'
    value = 0
  [../]
  [./homogeneous_v]
    type = DirichletBC
    variable = v
    boundary = '0 1 2 3'
    value = 0
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
  petsc_options_iname = '-snes_mf_operator -monolith_n_free_power -initial_snes_mf_operator  -monolith_set_sub_eigen -nlpower_set_sub_eigen'
  petsc_options_value = '1                  4                      1                          1                         1'
  petsc_options = '-initial_eps_monitor -snes_monitor -eps_monitor'
[]


[VectorPostprocessors]
  [./eigenvalues]
    type = Eigenvalues
    execute_on = 'timestep_end'
  [../]
[]

[Outputs]
  csv = true
  file_base = ne_deficient_b
  execute_on = 'timestep_end'
  [./console]
    type = Console
    outlier_variable_norms = false
  [../]
[]

# 16: eigenvalue: 0.03921472186593  0.039149639134272 0.03894906835492
# 64: 0.03897928685185  0.038975079139526 0.038961476514996 0.038961476514993
