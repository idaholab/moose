[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 100
  ny = 100
  xmin = -50
  xmax = 50
  ymin = -50
  ymax = 50
[]

[Variables]
  [./c_v]
  [../]
  [./jx_v]
  [../]
  [./jy_v]
  [../]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[ICs]
  [./Cv]
    type = ConstantIC
    value = 1e-6
    variable = c_v
  []
[]

[Debug]
  # show_var_residual = true
  show_var_residual_norms = true
[]

[AuxVariables]
  [grad_jx_x]
    order = FIRST
    family = MONOMIAL
  []
  [grad_jx_y]
    order = FIRST
    family = MONOMIAL
  []
  [grad_jy_y]
    order = FIRST
    family = MONOMIAL
  []
  [grad_jy_x]
    order = FIRST
    family = MONOMIAL
  []
[]

[AuxKernels]
  [flux_x_x]
    type = VariableGradientComponent
    gradient_variable = jx_v
    variable = grad_jx_x
    component = 'x'
  []
  [flux_x_y]
    type = VariableGradientComponent
    gradient_variable = jx_v
    variable = grad_jx_y
    component = 'y'
  []
  [flux_y_y]
    type = VariableGradientComponent
    gradient_variable = jy_v
    variable = grad_jy_y
    component = 'y'
  []
  [flux_y_x]
    type = VariableGradientComponent
    gradient_variable = jy_v
    variable = grad_jy_x
    component = 'x'
  []
[]

[Kernels]
  [./c]
    type = ADMatDiffusion
    variable = c_v
    diffusivity = D
  [../]
  [./time]
    type = TimeDerivative
    variable = c_v
  [../]
  [./flux_x]
    type = GenericGradientComponent #Only solves expression J = -DgradU (where D = 1)
    v = c_v
    variable = jx_v
    component = 0
    factor = 1
  [../]
  [./flux_y]
    type = GenericGradientComponent #Only solves expression J = -DgradU (where D = 1)
    v = c_v
    variable = jy_v
    component = 1
    factor = 1
  [../]
  [./TensorMechanics]
    displacements = 'disp_x disp_y'
  [../]
[]

[BCs]
  [left_x]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  []
  [Top_x]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
  [source_top_y]
    type = MatNeumannBC
    variable = c_v
    boundary = 'top'
    value = 1
    boundary_material = neumann_source_top_y
  []
  [source_right_x]
    type = MatNeumannBC
    variable = c_v
    boundary = 'right'
    value = 1
    boundary_material = neumann_source_right_x
  []
  [source_bottom_y]
    type = MatNeumannBC
    variable = c_v
    boundary = 'bottom'
    value = 1
    boundary_material = neumann_source_bottom_y
  []
  [source_left_x]
    type = MatNeumannBC
    variable = c_v
    boundary = 'left'
    value = 1
    boundary_material = neumann_source_left_x
  []
[]

[Materials]
  [./DiffusivityMat]
    type = ADGenericConstantMaterial
    prop_names = D
    prop_values = 1
  [../]
  [neumann_source_top_y] # +ve for entering
    type = ParsedMaterial
    expression = '1.5e-5' 
    property_name = neumann_source_top_y
    outputs = exodus
  []
  [neumann_source_right_x]
    type = ParsedMaterial
    expression = '-1.5e-5' #Sign Change 
    property_name = neumann_source_right_x
    outputs = exodus
  []
  [neumann_source_bottom_y] # +ve for entering
    type = ParsedMaterial
    expression = '1.5e-5' 
    property_name = neumann_source_bottom_y
    outputs = exodus
  []
  [neumann_source_left_x]
    type = ParsedMaterial
    expression = '-1.5e-5' #Sign Change 
    property_name = neumann_source_left_x
    outputs = exodus
  []
  #Creep strain increments
  [diffuse_strain_increment]
    type = DeviatoricStrainIncrement
    dimension = 2
    xflux = jx_v
    yflux = jy_v
    property_name = diffuse_strain
    output_properties = 'diffuse_strain'
    outputs = 'exodus'
  []
  [diffuse_creep_strain]
    type = SumTensorIncrements
    tensor_name = creep_strain
    coupled_tensor_increment_names = 'diffuse_strain'
    outputs = 'exodus'
  []
 [./strain]
    type = ComputeIncrementalStrain
    displacements = 'disp_x disp_y'
  [../]
  [stress]
    type = ComputeStrainIncrementBasedStress
    inelastic_strain_names = creep_strain
  []
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    C_ijkl = '120.0 80.0'
    fill_method = symmetric_isotropic
  [../]
[]

[Preconditioning]
  [./smp]
     type = SMP
     full = true
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  # petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  # petsc_options_value = 'lu superlu_dist'
  # petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart -pc_hypre_boomeramg_strong_threshold'
  # petsc_options_value = 'hypre    boomeramg      31                 0.7'
  # petsc_options_iname = '-pc_type -ksp_grmres_restart -sub_ksp_type -sub_pc_type -pc_asm_overlap'
  # petsc_options_value = 'asm      31                  preonly       lu           2'
  l_tol = 1e-4 #1e-3
  l_max_its = 5 # SK
  nl_max_its = 5 # SK
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-08 # SK
  end_time = 1e5
  dt = 0.1

  line_search = 'none' # SK
  automatic_scaling = false # SK
  nl_forced_its = 2 # SK

  # [Adaptivity]
  #   max_h_level = 2
  #   refine_fraction = 0.3
  #   coarsen_fraction = 0.2
  # []

  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1e-6
    iteration_window = 2
    optimal_iterations = 9
    growth_factor = 1.25
    cutback_factor = 0.8
  []
  dtmax = 1e5
  # end_time = 100
[]

[Outputs]
  exodus = true
  checkpoint = true
  file_base = Test
[]
