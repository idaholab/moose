[Mesh]
  type = GeneratedMesh
  dim = 2
  xmax = 100
  ymax = 70
  nx = 10
  ny = 7
  elem_type = QUAD4
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [rea]
    type = CoefReaction
    variable = u
    coefficient = 2.0
  []
  [rhs]
    type = CoefReaction
    variable = u
    coefficient = -1.0
    extra_vector_tags = 'eigen'
  []
[]

[BCs]
  [homogeneous]
    type = DirichletBC
    variable = u
    boundary = '0 1 2 3'
    value = 0
  []
  [eigen_bc]
    type = EigenDirichletBC
    variable = u
    boundary = '0 1 2 3'
  []
[]

[Problem]
  type = EigenProblem
[]

[Executioner]
  type = Eigenvalue
  eigen_problem_type = gen_non_hermitian
  which_eigen_pairs = smallest_magnitude
  n_eigen_pairs = 5
  n_basis_vectors = 18
  solve_type = jacobi_davidson
  eigen_tol = 1e-7
  output_all_eigenvectors = true
  normalization = bnorm
  normal_factor = 1
[]

[Postprocessors]
  [bnorm]
    type = EigenvectorBNorm
  []
  [umax_signed]
    type = NodalExtremeValue
    variable = u
    value_type = max_abs
    outputs = none
  []
  [umax]
    type = ParsedPostprocessor
    expression = 'abs(umax_signed)'
    pp_names = umax_signed
  []
[]

[VectorPostprocessors]
  [eigenvalues]
    type = Eigenvalues
    execute_on = 'timestep_end'
  []
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
  csv = true
[]
