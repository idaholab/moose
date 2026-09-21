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
  [stub_line]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 1
    xmin = 0.5
    xmax = 1.0
    elem_type = EDGE2
    subdomain_ids = '2'
    subdomain_name = 'stub'
  []
  [stub_raised]
    type = TransformGenerator
    input = stub_line
    transform = TRANSLATE
    vector_value = '0.0 0.5 0.3'
  []
  [stub_nodesets]
    type = RenameBoundaryGenerator
    input = stub_raised
    old_boundary = 'left right'
    new_boundary = 'reference_node stub_far_node'
  []
  [combined]
    type = CombinerGenerator
    inputs = 'plate stub_nodesets'
    avoid_merging_boundaries = true
  []
  [patch]
    type = ExtraNodesetGenerator
    input = combined
    new_boundary = 'patch'
    coord = '0.25 0.25 0.05; 0.75 0.25 0.05; 0.75 0.75 0.05; 0.25 0.75 0.05'
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
  [rot_x]
    block = 'stub'
  []
  [rot_y]
    block = 'stub'
  []
  [rot_z]
    block = 'stub'
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

[Physics/SolidMechanics/LineElement/QuasiStatic]
  [stub]
    block = 'stub'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    area = 1e-4
    Ay = 0.0
    Az = 0.0
    Iy = 1e-8
    Iz = 1e-8
    y_orientation = '0.0 1.0 0.0'
  []
[]

[Constraints]
  [rbe3]
    type = RBE3Constraint
    reference_boundary = 'reference_node'
    independent_boundaries = 'patch'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
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
  [stub_disp_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'stub_far_node'
    value = 0
  []
  [stub_disp_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'stub_far_node'
    value = 0
  []
  [stub_disp_z]
    type = DirichletBC
    variable = disp_z
    boundary = 'stub_far_node'
    value = 0
  []
  [stub_rot_x]
    type = DirichletBC
    variable = rot_x
    boundary = 'stub_far_node'
    value = 0
  []
  [stub_rot_y]
    type = DirichletBC
    variable = rot_y
    boundary = 'stub_far_node'
    value = 0
  []
  [stub_rot_z]
    type = DirichletBC
    variable = rot_z
    boundary = 'stub_far_node'
    value = 0
  []
  [stub_disp_x_eigen]
    type = EigenDirichletBC
    variable = disp_x
    boundary = 'stub_far_node'
  []
  [stub_disp_y_eigen]
    type = EigenDirichletBC
    variable = disp_y
    boundary = 'stub_far_node'
  []
  [stub_disp_z_eigen]
    type = EigenDirichletBC
    variable = disp_z
    boundary = 'stub_far_node'
  []
  [stub_rot_x_eigen]
    type = EigenDirichletBC
    variable = rot_x
    boundary = 'stub_far_node'
  []
  [stub_rot_y_eigen]
    type = EigenDirichletBC
    variable = rot_y
    boundary = 'stub_far_node'
  []
  [stub_rot_z_eigen]
    type = EigenDirichletBC
    variable = rot_z
    boundary = 'stub_far_node'
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
  [stub_elasticity]
    type = ComputeElasticityBeam
    block = 'stub'
    youngs_modulus = 68
    poissons_ratio = 0.36
    shear_coefficient = 1.0
  []
  [stub_stress]
    type = ComputeBeamResultants
    block = 'stub'
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
