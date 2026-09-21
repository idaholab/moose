[Mesh]
  [plate]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 4
    ny = 4
    nz = 1
    xmin = 0.0
    xmax = 1.0
    ymin = 0.0
    ymax = 1.0
    zmin = 0.0
    zmax = 0.05
    elem_type = HEX8
    subdomain_ids = '1'
    subdomain_name = 'plate'
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
    type = ADReaction
    variable = disp_x
    block = 'plate'
    extra_vector_tags = 'eigen'
    rate = -2.7e3
  []
  [mass_y]
    type = ADReaction
    variable = disp_y
    block = 'plate'
    extra_vector_tags = 'eigen'
    rate = -2.7e3
  []
  [mass_z]
    type = ADReaction
    variable = disp_z
    block = 'plate'
    extra_vector_tags = 'eigen'
    rate = -2.7e3
  []
  [stiffness_x]
    type = StressDivergenceTensors
    variable = disp_x
    block = 'plate'
    component = 0
  []
  [stiffness_y]
    type = StressDivergenceTensors
    variable = disp_y
    block = 'plate'
    component = 1
  []
  [stiffness_z]
    type = StressDivergenceTensors
    variable = disp_z
    block = 'plate'
    component = 2
  []
[]

[BCs]
  [clamp_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'left'
    value = 0
  []
  [clamp_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'left'
    value = 0
  []
  [clamp_z]
    type = DirichletBC
    variable = disp_z
    boundary = 'left'
    value = 0
  []
  [clamp_x_eigen]
    type = EigenDirichletBC
    variable = disp_x
    boundary = 'left'
  []
  [clamp_y_eigen]
    type = EigenDirichletBC
    variable = disp_y
    boundary = 'left'
  []
  [clamp_z_eigen]
    type = EigenDirichletBC
    variable = disp_z
    boundary = 'left'
  []
[]

[Materials]
  [plate_elasticity]
    type = ComputeIsotropicElasticityTensor
    block = 'plate'
    youngs_modulus = 68e9
    poissons_ratio = 0.36
  []
  [plate_strain]
    type = ComputeSmallStrain
    block = 'plate'
  []
  [plate_stress]
    type = ComputeLinearElasticStress
    block = 'plate'
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
  # The Newton phase is one SNES solve whose residual falls by about a factor of 1.5 per
  # iteration because the plate's first two modes are close, while the eigenvalue itself is
  # settled to twelve digits after five. At the default 1e-8 the solve needs about thirty
  # iterations and round-off at this stiffness (68 GPa) can stop it short on some platforms.
  nl_rel_tol = 1e-6
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
