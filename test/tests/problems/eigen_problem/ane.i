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

# the minimum eigenvalue is (2*PI*(p-1)^(1/p)/a/p/sin(PI/p))^p;
# Its inverse is 35.349726539758187. Here a is equal to 10.

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]


[Kernels]
  [./diff]
    type = PHarmonic
    variable = u
    p = 3
  [../]

  [./rhs]
    type = PMassKernel
    eigen_kernel = true
    variable = u
    p = 3
  [../]
[]

[BCs]
  [./homogeneous]
    type = DirichletBC
    variable = u
    boundary = '0 2'
    value = 0
  [../]
[]

[Problem]
  type = EigenProblem
  eigen_problem_type = gen_non_hermitian
  n_eigen_pairs = 1
  n_basis_vectors = 18
  which_eigen_pairs = TARGET_MAGNITUDE
[]


[Executioner]
  type = Steady
  eigen_solve_type = MONOLITH_NEWTON
  petsc_options_iname = '-eps_power_snes_mf_operator'
  petsc_options_value = '1'
[]


[VectorPostprocessors]
  [./eigenvalues]
    type = Eigenvalues
    execute_on = 'timestep_end'
  [../]
[]

[Outputs]
  csv = true
  file_base = ane
  execute_on = 'timestep_end'
  [./console]
    type = Console
    outlier_variable_norms = false
  [../]
[]
