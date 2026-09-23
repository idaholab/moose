[Mesh]
  allow_renumbering = false
  [solid]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 10
    ny = 3
    nz = 3
    xmin = 0.0
    xmax = 5.0
    ymin = 0.0
    ymax = 1.0
    zmin = 0.0
    zmax = 1.0
    elem_type = HEX8
    subdomain_ids = '1'
    subdomain_name = 'solid'
  []
  [solid_tie_face]
    type = RenameBoundaryGenerator
    input = solid
    old_boundary = 'right'
    new_boundary = 'tie_face'
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
  [mass_x]
    type = ADReaction
    variable = disp_x
    extra_vector_tags = 'eigen'
    rate = -1.0
  []
  [mass_y]
    type = ADReaction
    variable = disp_y
    extra_vector_tags = 'eigen'
    rate = -1.0
  []
  [mass_z]
    type = ADReaction
    variable = disp_z
    extra_vector_tags = 'eigen'
    rate = -1.0
  []
[]

[NodalKernels]
  [tip_mass_x]
    type = ReactionNodalKernel
    variable = disp_x
    boundary = 'tie_face'
    coeff = -0.0625
    extra_vector_tags = 'eigen'
  []
  [tip_mass_y]
    type = ReactionNodalKernel
    variable = disp_y
    boundary = 'tie_face'
    coeff = -0.0625
    extra_vector_tags = 'eigen'
  []
  [tip_mass_z]
    type = ReactionNodalKernel
    variable = disp_z
    boundary = 'tie_face'
    coeff = -0.0625
    extra_vector_tags = 'eigen'
  []
[]

[BCs]
  [fix_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'left'
    value = 0.0
  []
  [fix_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'left'
    value = 0.0
  []
  [fix_z]
    type = DirichletBC
    variable = disp_z
    boundary = 'left'
    value = 0.0
  []
  [fix_x_eigen]
    type = EigenDirichletBC
    variable = disp_x
    boundary = 'left'
  []
  [fix_y_eigen]
    type = EigenDirichletBC
    variable = disp_y
    boundary = 'left'
  []
  [fix_z_eigen]
    type = EigenDirichletBC
    variable = disp_z
    boundary = 'left'
  []
[]

[Materials]
  [elasticity]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1.0e6
    poissons_ratio = 0.3
  []
  [strain]
    type = ComputeSmallStrain
  []
  [stress]
    type = ComputeLinearElasticStress
  []
[]

[Problem]
  type = EigenProblem
  active_eigen_index = 0
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Eigenvalue
  solve_type = NEWTON
  n_eigen_pairs = 1
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu mumps'
  eigen_tol = 1e-8
  nl_abs_tol = 1e-10
[]

[VectorPostprocessors]
  [omega_squared]
    type = Eigenvalues
    execute_on = 'timestep_end'
  []
[]

[Outputs]
  csv = true
  execute_on = 'timestep_end'
[]
