index = 0

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    elem_type = HEX8
    dim = 3
    xmin = 0
    xmax = 1.0
    nx = 10
    ymin = 0
    ymax = 0.1
    ny = 1
    zmin = 0
    zmax = 0.15
    nz = 2
  []
[]
[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[Kernels]
  [mass_x]
    type = ADCoefReaction
    variable = disp_x
    extra_vector_tags = 'eigen'
    coefficient = -2.7e3
  []
  [mass_y]
    type = ADCoefReaction
    variable = disp_y
    extra_vector_tags = 'eigen'
    coefficient = -2.7e3
  []
  [mass_z]
    type = ADCoefReaction
    variable = disp_z
    extra_vector_tags = 'eigen'
    coefficient = -2.7e3
  []
  [stiffness_x]
    type = StressDivergenceTensors
    variable = disp_x
    component = 0
  []
  [stiffness_y]
    type = StressDivergenceTensors
    variable = disp_y
    component = 1
  []
  [stiffness_z]
    type = StressDivergenceTensors
    variable = disp_z
    component = 2
  []
[]

[BCs]
  [dirichlet_x]
    type = DirichletBC
    variable = disp_x
    value = 0
    boundary = 'left'
  []
  [dirichlet_y]
    type = DirichletBC
    variable = disp_y
    value = 0
    boundary = 'left'
  []
  [dirichlet_z]
    type = DirichletBC
    variable = disp_z
    value = 0
    boundary = 'left'
  []
  [dirichlet_x_e]
    type = EigenDirichletBC
    variable = disp_x
    boundary = 'left'
  []
  [dirichlet_y_e]
    type = EigenDirichletBC
    variable = disp_y
    boundary = 'left'
  []
  [dirichlet_z_e]
    type = EigenDirichletBC
    variable = disp_z
    boundary = 'left'
  []
[]

[Materials]
  [elastic_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 68e9
    poissons_ratio = 0.36
  []
  [compute_stress]
    type = ComputeLinearElasticStress
  []
  [compute_strain]
    type = ComputeSmallStrain
  []

[]
[Executioner]
  type = Eigenvalue
  solve_type = KRYLOVSCHUR
  which_eigen_pairs = SMALLEST_MAGNITUDE
  n_eigen_pairs = 2
  n_basis_vectors = 5
  petsc_options = '-eps_monitor_all -eps_view'
  petsc_options_iname = '-st_type -eps_target -st_pc_type -st_pc_factor_mat_solver_type'
  petsc_options_value = 'sinvert 0 lu mumps'
  eigen_tol = 1e-8
[]

[VectorPostprocessors]
  [omega_squared]
    type = Eigenvalues
    execute_on = TIMESTEP_END
  []

[]

[Problem]
  type = EigenProblem
  active_eigen_index = ${index}
[]

[Outputs]
  csv = true
  exodus = true
  execute_on = 'timestep_end'
[]

