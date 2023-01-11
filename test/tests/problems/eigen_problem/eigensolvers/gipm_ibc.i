[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 100
  ymin = 0
  ymax = 100
  elem_type = QUAD4
  nx = 64
  ny = 64

  displacements = 'x_disp y_disp'
[]

[Variables]
  [./u]
    order = first
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./x_disp]
  [../]
  [./y_disp]
  [../]
[]

[AuxKernels]
  [./x_disp]
    type = FunctionAux
    variable = x_disp
    function = x_disp_func
  [../]
  [./y_disp]
    type = FunctionAux
    variable = y_disp
    function = y_disp_func
  [../]
[]

[Functions]
  [./x_disp_func]
    type = ParsedFunction
    expression = 0
  [../]
  [./y_disp_func]
    type = ParsedFunction
    expression = 0
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
    use_displaced_mesh = true
  [../]

  [./rea]
    type = CoefReaction
    variable = u
    coefficient = 2.0
    use_displaced_mesh = true
  [../]

  [./rhs]
    type = CoefReaction
    variable = u
    use_displaced_mesh = true
    coefficient = -1.0
    extra_vector_tags = 'eigen'
  [../]
[]

[BCs]
  [./nbc_homogeneous]
    type = DirichletBC
    variable = u
    boundary = '0'
    value = 0
    use_displaced_mesh = true
  [../]

  [./nbc_eigen]
    type = EigenDirichletBC
    variable = u
    boundary = '0'
    use_displaced_mesh = true
  [../]

  [./ibc]
    type = VacuumBC
    variable = u
    boundary = '1 2 3'
    extra_vector_tags = 'eigen'
    use_displaced_mesh = true
  []
[]

[Executioner]
  type = Eigenvalue
  eigen_problem_type = gen_non_hermitian
  which_eigen_pairs = SMALLEST_MAGNITUDE
  n_eigen_pairs = 1
  n_basis_vectors = 18
  solve_type = jacobi_davidson
  petsc_options = '-eps_view'
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
  [./console]
    type = Console
    outlier_variable_norms = false
  [../]
[]
