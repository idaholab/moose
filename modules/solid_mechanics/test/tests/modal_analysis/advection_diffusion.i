[Mesh]
  type = GeneratedMesh
  dim = 1
  xmax = 1
  nx = 10
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [adv]
    type = ConservativeAdvection
    variable = u
    velocity_material = velocity
  []
[]

[Materials]
  [velocity]
    type = GenericConstantVectorMaterial
    prop_names = 'velocity'
    prop_values = '50 0 0'
  []
[]

[BCs]
  [homogeneous]
    type = DirichletBC
    variable = u
    boundary = 'left right'
    value = 0
  []
[]

[Executioner]
  type = Eigenvalue
  which_eigen_pairs = largest_magnitude
  eigen_problem_type = NON_HERMITIAN
  n_eigen_pairs = 2
  n_basis_vectors = 6
  solve_type = krylovschur
[]

[VectorPostprocessors]
  [eigenvalues]
    type = NaturalFrequencies
  []
[]

[Outputs]
  csv = true
[]
