#This test demonstrates the volumetric strain increments under both lattice site conserved and non-conserved conditions.
#VolumeStrainIncrement class in this test has been inherited from FluxBasedStrainIncrement object. 
#For this test, both the deviatoric part of strain rate tensor (calculated by DeviatoricStrainIncrement) and volumetric strain is added using SumTensorIncrement object. 
#A symmetrical geometry is considered with a constant source of vacancies. 

[Mesh]
  [annular]
    type = AnnularMeshGenerator
    nr = 100
    rmin = 0
    rmax = 50
    nt = 100
    dmin = 0
    dmax = 360
  []
[]

[Variables]
  [jx_v]
  []
  [jy_v]
  []
  [c_v]
  []
  [disp_x]
  []
  [disp_y]
  []
[]

#Test
[AuxVariables]
  [u]
  []
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

[ICs]
  [./cv]
    type = ConstantIC
    value = 1e-6
    variable = c_v
  []
  [./MaskingCondition]
    type = SmoothCircleIC
    invalue = 1
    outvalue = 0
    radius = 25
    variable = u
    x1 = 0
    y1 = 0
  [../]
[]

[Debug]
  # show_var_residual = true
  show_var_residual_norms = true
[]

[Kernels]
  [./c]
    type = ADMatDiffusion
    variable = c_v
    diffusivity = D
  [../]
  [dc_dt]
    type = TimeDerivative
    variable = c_v
  []
  #Source term
  [./MaskedBodyforce]
    type = MaskedBodyForce
    mask = mask
    variable = c_v
    value = 1e-5
  []
  [./flux_x]
    type = GenericGradientComponent #Only solves expression J = -DgradU (where D = 1 (given by factor option))
    v = c_v
    variable = jx_v
    component = 0
    factor = 1
  [../]
  [./flux_y]
    type = GenericGradientComponent #Only solves expression J = -DgradU (where D = 1 (given by factor option))
    v = c_v
    variable = jy_v
    component = 1
    factor = 1
  [../]
  #Tensor mechanics
  [TensorMechanics]
    displacements = 'disp_x disp_y'
  []
[]

#Test
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

[Materials]
  [./Lambda]
    type = GenericConstantMaterial
    prop_names = 'Lambda_J Lambda_P'
    prop_values = '1 1'
  []
  [./Source]
    type = ParsedMaterial
    property_name = source
    expression = 'mask*1e-5'
    material_property_names = 'mask'
  []
  [./Mask]
    type = ParsedMaterial
    expression = if(u>0,0,1)
    property_name = mask
    coupled_variables = u
    output_properties = 'mask'
    outputs = 'exodus'
  []
  [./DiffusivityMat]
    type = ADGenericConstantMaterial
    prop_names = D
    prop_values = 1
  [../]
  #Creep strain increments
  [deviatoric_strain_increment]
    type = DeviatoricStrainIncrement
    dimension = 2
    xflux = jx_v
    yflux = jy_v
    property_name = deviatoric_strain
    output_properties = 'deviatoric_strain'
    outputs = 'exodus'
  []
  [volumetric_strain_increment]
    type = VolumeStrainIncrement
    xflux = jx_v
    yflux = jy_v
    Lambda_Prefactor_J = Lambda_J
    Lambda_Prefactor_P = Lambda_P
    Source = source
    property_name = volumetric_strain
    output_properties = 'volumetric_strain'
    outputs = 'exodus'
  []
  [diffuse_creep_strain]
    type = SumTensorIncrements
    tensor_name = creep_strain
    coupled_tensor_increment_names = 'deviatoric_strain volumetric_strain'
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
  [SMP]
    type = SMP
    full = true
  []
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
  file_base = Test_volumetric
[]
