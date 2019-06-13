[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 10
  ny = 10
  elem_type = QUAD4
[]

[Variables]
  [./u]
    order = FIRST
    family = MONOMIAL
  [../]
[]

[DGKernels]
  [./dg_diff]
    type = DGDiffusion
    variable = u
    sigma = 6
    epsilon = -1
  []
[]

[Kernels]
  [./rhs]
    type = Reaction
    variable = u
    extra_vector_tags = 'eigen'
  [../]
[]

[Executioner]
  type = Eigenvalue
  solve_type = KRYLOVSCHUR
  eigen_problem_type = HERMITIAN
  which_eigen_pairs = LARGEST_MAGNITUDE
[]

[VectorPostprocessors]
  [./eigenvalues]
    type = Eigenvalues
    execute_on = 'timestep_end'
  [../]
[]

[Outputs]
  csv = true
  execute_on = 'timestep_end'
[]
