[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 8
    xmin = 0.0
    xmax = 1.0
    ymin = 0.0
    ymax = 2.0
    elem_type = QUAD4
    subdomain_ids = '1'
    subdomain_name = 'block'
  []
  [rotate]
    type = TransformGenerator
    transform = ROTATE
    vector_value = '0 0 30'
    input = square
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
[]

[Kernels]
  [mass_x]
    type = ADReaction
    variable = disp_x
    block = 'block'
    extra_vector_tags = 'eigen'
    rate = -2.7e3
  []
  [mass_y]
    type = ADReaction
    variable = disp_y
    block = 'block'
    extra_vector_tags = 'eigen'
    rate = -2.7e3
  []
  [stiffness_x]
    type = StressDivergenceTensors
    variable = disp_x
    block = 'block'
    component = 0
  []
  [stiffness_y]
    type = StressDivergenceTensors
    variable = disp_y
    block = 'block'
    component = 1
  []
[]

[BCs]
  inactive = 'roller_x roller_y roller_x_eigen roller_y_eigen InclinedNoDisplacementBC'
  [roller_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'left'
    value = 0
  []
  [roller_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'bottom'
    value = 0
  []
  [roller_x_eigen]
    type = EigenDirichletBC
    variable = disp_x
    boundary = 'left'
  []
  [roller_y_eigen]
    type = EigenDirichletBC
    variable = disp_y
    boundary = 'bottom'
  []
  [InclinedNoDisplacementBC]
    [bottom]
      boundary = bottom
      penalty = 1.0e6
      displacements = 'disp_x disp_y'
    []
    [left]
      boundary = left
      penalty = 1.0e6
      displacements = 'disp_x disp_y'
    []
  []
[]

[Constraints]
  [bottom]
    type = InclinedNoDisplacementConstraint
    boundary = bottom
    displacements = 'disp_x disp_y'
  []
  [left]
    type = InclinedNoDisplacementConstraint
    boundary = left
    displacements = 'disp_x disp_y'
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    block = 'block'
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  []
  [strain]
    type = ComputeSmallStrain
    block = 'block'
  []
  [stress]
    type = ComputeLinearElasticStress
    block = 'block'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Eigenvalue
  solve_type = newton
  n_eigen_pairs = 1
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu mumps'
  eigen_tol = 1e-8
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
