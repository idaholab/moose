[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    elem_type = EDGE2
    dim = 1
    xmin = 0
    xmax = 1.0
    nx = 40
  []
[]

[GlobalParams]
  displacements = 'disp_x'
[]

[Variables]
  [disp_x]
  []
[]

[Kernels]
  [mass_x]
    type = ADMatReaction
    variable = disp_x
    reaction_rate = density
    extra_vector_tags = 'eigen'
  []
  [stiffness_x]
    type = StressDivergenceTensors
    variable = disp_x
    component = 0
  []
[]

[BCs]
  [dirichlet_left]
    type = DirichletBC
    variable = disp_x
    value = 0
    boundary = 'left'
  []
  [dirichlet_right]
    type = DirichletBC
    variable = disp_x
    value = 0
    boundary = 'right'
  []
  [dirichlet_left_e]
    type = EigenDirichletBC
    variable = disp_x
    boundary = 'left'
  []
  [dirichlet_right_e]
    type = EigenDirichletBC
    variable = disp_x
    boundary = 'right'
  []
[]

[Materials]
  [elastic_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 68e9
    poissons_ratio = 0
  []
  [compute_strain]
    type = ComputeSmallStrain
  []
  [compute_stress]
    type = ComputeLinearElasticStress
  []
  [density]
    type = ADGenericConstantMaterial
    prop_names = density
    prop_values = 2700
  []
[]

[Executioner]
  type = Eigenvalue
  solve_type = KRYLOVSCHUR
  which_eigen_pairs = SMALLEST_MAGNITUDE
  n_eigen_pairs = 3
  n_basis_vectors = 10
  petsc_options_iname = '-st_type -eps_target -st_pc_type -st_pc_factor_mat_solver_type'
  petsc_options_value = 'sinvert 0 lu mumps'
  eigen_tol = 1e-8
[]

[VectorPostprocessors]
  [eigenvalues]
    type = Eigenvalues
    natural_frequency = true
    execute_on = TIMESTEP_END
  []
[]

[Problem]
  type = EigenProblem
[]

[Outputs]
  csv = true
  execute_on = 'timestep_end'
[]
