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
  [stub_line]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 1
    xmin = 5.0
    xmax = 6.0
    elem_type = EDGE2
    subdomain_ids = '2'
    subdomain_name = 'stub'
  []
  [stub_moved]
    type = TransformGenerator
    input = stub_line
    transform = TRANSLATE
    vector_value = '0.0 0.5 0.5'
  []
  [stub_nodesets]
    type = RenameBoundaryGenerator
    input = stub_moved
    old_boundary = 'left right'
    new_boundary = 'stub_first_node stub_far_node'
  []
  [combined]
    type = CombinerGenerator
    inputs = 'solid_tie_face stub_nodesets'
    avoid_merging_boundaries = true
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
  [stiffness_x]
    type = StressDivergenceTensors
    variable = disp_x
    component = 0
    block = 'solid'
  []
  [stiffness_y]
    type = StressDivergenceTensors
    variable = disp_y
    component = 1
    block = 'solid'
  []
  [stiffness_z]
    type = StressDivergenceTensors
    variable = disp_z
    component = 2
    block = 'solid'
  []
  [mass_x]
    type = ADReaction
    variable = disp_x
    block = 'solid'
    extra_vector_tags = 'eigen'
    rate = -1.0
  []
  [mass_y]
    type = ADReaction
    variable = disp_y
    block = 'solid'
    extra_vector_tags = 'eigen'
    rate = -1.0
  []
  [mass_z]
    type = ADReaction
    variable = disp_z
    block = 'solid'
    extra_vector_tags = 'eigen'
    rate = -1.0
  []
  [stub_disp_x]
    type = Reaction
    variable = disp_x
    block = 'stub'
    rate = 1.0e-6
  []
  [stub_disp_y]
    type = Reaction
    variable = disp_y
    block = 'stub'
    rate = 1.0e-6
  []
  [stub_disp_z]
    type = Reaction
    variable = disp_z
    block = 'stub'
    rate = 1.0e-6
  []
  [stub_rot_x]
    type = Reaction
    variable = rot_x
    block = 'stub'
    rate = 1.0e-6
  []
  [stub_rot_y]
    type = Reaction
    variable = rot_y
    block = 'stub'
    rate = 1.0e-6
  []
  [stub_rot_z]
    type = Reaction
    variable = rot_z
    block = 'stub'
    rate = 1.0e-6
  []
[]

[NodalKernels]
  [tip_mass_x]
    type = ReactionNodalKernel
    variable = disp_x
    boundary = 'stub_first_node'
    coeff = -1.0
    extra_vector_tags = 'eigen'
  []
  [tip_mass_y]
    type = ReactionNodalKernel
    variable = disp_y
    boundary = 'stub_first_node'
    coeff = -1.0
    extra_vector_tags = 'eigen'
  []
  [tip_mass_z]
    type = ReactionNodalKernel
    variable = disp_z
    boundary = 'stub_first_node'
    coeff = -1.0
    extra_vector_tags = 'eigen'
  []
[]

[Constraints]
  [rbe2]
    type = RBE2Constraint
    independent_boundary = 'stub_first_node'
    dependent_boundary = 'tie_face'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    dependent_dofs = translations
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
  [stub_fix_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'stub_far_node'
    value = 0.0
  []
  [stub_fix_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'stub_far_node'
    value = 0.0
  []
  [stub_fix_z]
    type = DirichletBC
    variable = disp_z
    boundary = 'stub_far_node'
    value = 0.0
  []
  [stub_fix_rot_x]
    type = DirichletBC
    variable = rot_x
    boundary = 'stub_far_node'
    value = 0.0
  []
  [stub_fix_rot_y]
    type = DirichletBC
    variable = rot_y
    boundary = 'stub_far_node'
    value = 0.0
  []
  [stub_fix_rot_z]
    type = DirichletBC
    variable = rot_z
    boundary = 'stub_far_node'
    value = 0.0
  []
  [stub_fix_x_eigen]
    type = EigenDirichletBC
    variable = disp_x
    boundary = 'stub_far_node'
  []
  [stub_fix_y_eigen]
    type = EigenDirichletBC
    variable = disp_y
    boundary = 'stub_far_node'
  []
  [stub_fix_z_eigen]
    type = EigenDirichletBC
    variable = disp_z
    boundary = 'stub_far_node'
  []
  [stub_fix_rot_x_eigen]
    type = EigenDirichletBC
    variable = rot_x
    boundary = 'stub_far_node'
  []
  [stub_fix_rot_y_eigen]
    type = EigenDirichletBC
    variable = rot_y
    boundary = 'stub_far_node'
  []
  [stub_fix_rot_z_eigen]
    type = EigenDirichletBC
    variable = rot_z
    boundary = 'stub_far_node'
  []
[]

[Materials]
  [elasticity]
    type = ComputeIsotropicElasticityTensor
    block = 'solid'
    youngs_modulus = 1.0e6
    poissons_ratio = 0.3
  []
  [strain]
    type = ComputeSmallStrain
    block = 'solid'
  []
  [stress]
    type = ComputeLinearElasticStress
    block = 'solid'
  []
  [stub_placeholder]
    type = GenericConstantMaterial
    block = 'stub'
    prop_names = 'stub_unused'
    prop_values = '0.0'
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
