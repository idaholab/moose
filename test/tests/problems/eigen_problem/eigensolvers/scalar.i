[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 1
  nx = 1
[]

[Variables]
  [./f1]
    family = SCALAR
    order = FIRST
    initial_condition = 1
  [../]
  [./f2]
    family = SCALAR
    order = FIRST
    initial_condition = 1
  [../]
[]

[ScalarKernels]
  [./row1]
    type = ParsedODEKernel
    variable = f1
    expression = '5*f1 + 2*f2'
    coupled_variables = 'f2'
  [../]

  [./row2]
    type = ParsedODEKernel
    variable = f2
    expression = '2*f1 + 5*f2'
    coupled_variables = 'f1'
  [../]
[]

[VectorPostprocessors]
  [./eigenvalues]
    type = Eigenvalues
    execute_on = 'timestep_end'
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Eigenvalue
  which_eigen_pairs = LARGEST_MAGNITUDE
  eigen_problem_type = HERMITIAN
  n_eigen_pairs = 2
  n_basis_vectors = 4
  eigen_max_its = 10
  solve_type = KRYLOVSCHUR
  petsc_options = '-eps_view'
[]

[Outputs]
  csv = true
[]
