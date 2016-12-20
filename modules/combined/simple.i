#
# Eigenstrain with neighbor relation of quasiperiodicity
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
  xmin = -0.5
  xmax = 0.5

  ymin = -0.5
  ymax = 0.5

[]

[UserObjects]
  [./quasiperiodicneighbors_x]
    type = QuasiPeriodicNeighbors
    execute_on = 'initial linear nonlinear timestep_begin'
    component = 0
  [../]
  [./quasiperiodicneighbors_y]
    type = QuasiPeriodicNeighbors
    execute_on = 'initial linear nonlinear timestep_begin'
    component = 1
  [../]
[]

[MeshModifiers]
  [./cnode]
    type = AddExtraNodeset
    coord = '0.0 0.0'
    new_boundary = 100
  [../]
  [./anode]
    type = AddExtraNodeset
    coord = '0.0 0.5'
    new_boundary = 101
  [../]
[]

[GlobalParams]
  derivative_order = 2
  enable_jit = true
  displacements = 'disp_x disp_y'
  use_displaced_mesh = false
  jacobian_fill = 0
[]

[Variables]
  # Mesh displacement
  [./disp_x]
  [../]
  [./disp_y]
  [../]

  # Lagrange multipliers
  [./lagrange_e_xx_lr]
  [../]
  [./lagrange_e_xy_lr]
  [../]
  [./lagrange_e_yy_lr]
  [../]

  [./lagrange_e_xx_tb]
  [../]
  [./lagrange_e_xy_tb]
  [../]
  [./lagrange_e_yy_tb]
  [../]
[]

# AuxVars to compute the free energy density for outputting
[AuxVariables]
  [./c]
  [../]
[]

[AuxKernels]
  [./c]
    type = FunctionAux
    variable = c
    function = 'sin(2*x*pi)*sin(2*y*pi)/2*sin(t+1.5)^2+0.5'
  [../]
[]

[Kernels]
  # Set up stress divergence kernels
  [./TensorMechanics]
  [../]

  [./null_l_xx_lr]
    type = NullKernel
    variable = lagrange_e_xx_lr
  [../]
  [./null_l_xy_lr]
    type = NullKernel
    variable = lagrange_e_xy_lr
  [../]
  [./null_l_yy_lr]
    type = NullKernel
    variable = lagrange_e_yy_lr
  [../]

  [./null_l_xx_tb]
    type = NullKernel
    variable = lagrange_e_xx_tb
  [../]
  [./null_l_xy_tb]
    type = NullKernel
    variable = lagrange_e_xy_tb
  [../]
  [./null_l_yy_tb]
    type = NullKernel
    variable = lagrange_e_yy_tb
  [../]
[]

[Materials]
  [./shear1]
    type = GenericConstantRankTwoTensor
    block = 0
    tensor_values = '0 0 0 0 0 0.5'
    tensor_name = shear1
  [../]
  [./shear2]
    type = GenericConstantRankTwoTensor
    block = 0
    tensor_values = '0 0 0 0 0 -0.5'
    tensor_name = shear2
  [../]
  [./expand3]
    type = GenericConstantRankTwoTensor
    block = 0
    tensor_values = '1 1 0 0 0 0'
    tensor_name = expand3
  [../]

  [./weight1]
    type = DerivativeParsedMaterial
    block = 0
    function = '0.3*c^2'
    f_name = weight1
    args = c
  [../]
  [./weight2]
    type = DerivativeParsedMaterial
    block = 0
    function = '0.3*(1-c)^2'
    f_name = weight2
    args = c
  [../]
  [./weight3]
    type = DerivativeParsedMaterial
    block = 0
    function = '4*(0.5-c)^2'
    f_name = weight3
    args = c
  [../]

  # matrix phase
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    block = 0
    C_ijkl = '1 1'
    fill_method = symmetric_isotropic
  [../]
  [./strain]
    type = ComputeSmallStrain
    block = 0
    displacements = 'disp_x disp_y'
    eigenstrain_names = eigenstrain
  [../]

  [./eigenstrain]
    type = CompositeEigenstrain
    block = 0
    tensors = 'shear1  shear2  expand3'
    weights = 'weight1 weight2 weight3'
    args = c
    eigenstrain_name = eigenstrain
  [../]

  [./stress]
    type = ComputeLinearElasticStress
    block = 0
  [../]
[]

[InterfaceKernels]
  #[./xx_int_left]
  #  type = QuasiPeriodicTestKernel
  #  variable = disp_x
  #  neighbor_var = disp_x
  #  component = 0
  #  boundary = 'left'
  #[../]
  #
  #[./yx_int_left]
  #  type = QuasiPeriodicTestKernel
  #  variable = disp_y
  #  neighbor_var = disp_y
  #  component = 0
  #  boundary = 'left'
  #[../]
  #
  ### -----top-----
  #[./xy_int_top]
  #  type = QuasiPeriodicTestKernel
  #  variable = disp_x
  #  neighbor_var = disp_x
  #  component = 1
  #  boundary = 'top'
  #[../]
  #
  #[./yy_int_top]
  #  type = QuasiPeriodicTestKernel
  #  variable = disp_y
  #  neighbor_var = disp_y
  #  component = 1
  #  boundary = 'top'
  #[../]


#----------lagrange multiplier interface kernels

  #[./iface_x_lr]
  #  type = InterfaceDiffusionBoundaryTerm
  #  boundary = 'left'
  #  variable = disp_x
  #  neighbor_var = disp_x
  #[../]
  [./lambda_xx_lr]
    type = EqualGradientLagrangeMultiplier
    variable = lagrange_e_xx_lr
    boundary = 'left'
    element_var = disp_x
    neighbor_var = disp_x
    component = 0
  [../]
  [./constraint_xx_lr]
    type = EqualGradientLagrangeInterface
    boundary = 'left'
    lambda = lagrange_e_xx_lr
    variable = disp_x
    neighbor_var = disp_x
    component = 0
  [../]

  #[./iface_x_lr]
  #  type = InterfaceDiffusionBoundaryTerm
  #  boundary = 'left'
  #  variable = disp_x
  #  neighbor_var = disp_x
  #[../]
  [./lambda_xy_lr]
    type = EqualGradientLagrangeMultiplier
    variable = lagrange_e_xy_lr
    boundary = 'left'
    element_var = disp_x
    neighbor_var = disp_x
    component = 1
  [../]
  [./constraint_xy_lr]
    type = EqualGradientLagrangeInterface
    boundary = 'left'
    lambda = lagrange_e_xy_lr
    variable = disp_x
    neighbor_var = disp_x
    component = 1
  [../]

  #[./iface_x_lr]
  #  type = InterfaceDiffusionBoundaryTerm
  #  boundary = 'left'
  #  variable = disp_x
  #  neighbor_var = disp_x
  #[../]
  [./lambda_yy_lr]
    type = EqualGradientLagrangeMultiplier
    variable = lagrange_e_yy_lr
    boundary = 'left'
    element_var = disp_y
    neighbor_var = disp_y
    component = 1
  [../]
  [./constraint_yy_lr]
    type = EqualGradientLagrangeInterface
    boundary = 'left'
    lambda = lagrange_e_yy_lr
    variable = disp_y
    neighbor_var = disp_y
    component = 1
  [../]


  #[./iface_y_tb]
  #  type = InterfaceDiffusionBoundaryTerm
  #  boundary = 'top'
  #  variable = disp_y
  #  neighbor_var = disp_y
  #[../]
  [./lambda_yy_tb]
    type = EqualGradientLagrangeMultiplier
    variable = lagrange_e_yy_tb
    boundary = 'top'
    element_var = disp_y
    neighbor_var = disp_y
    component = 1
  [../]
  [./constraint_yy_tb]
    type = EqualGradientLagrangeInterface
    boundary = 'top'
    lambda = lagrange_e_yy_tb
    variable = disp_y
    neighbor_var = disp_y
    component = 1
  [../]

  #[./iface_y_tb]
  #  type = InterfaceDiffusionBoundaryTerm
  #  boundary = 'top'
  #  variable = disp_y
  #  neighbor_var = disp_y
  #[../]
  [./lambda_yx_tb]
    type = EqualGradientLagrangeMultiplier
    variable = lagrange_e_xy_tb
    boundary = 'top'
    element_var = disp_y
    neighbor_var = disp_y
    component = 0
  [../]
  [./constraint_yx_tb]
    type = EqualGradientLagrangeInterface
    boundary = 'top'
    lambda = lagrange_e_xy_tb
    variable = disp_y
    neighbor_var = disp_y
    component = 0
  [../]

  #[./iface_y_tb]
  #  type = InterfaceDiffusionBoundaryTerm
  #  boundary = 'top'
  #  variable = disp_y
  #  neighbor_var = disp_y
  #[../]
  [./lambda_xx_tb]
    type = EqualGradientLagrangeMultiplier
    variable = lagrange_e_xx_tb
    boundary = 'top'
    element_var = disp_x
    neighbor_var = disp_x
    component = 0
  [../]
  [./constraint_xx_tb]
    type = EqualGradientLagrangeInterface
    boundary = 'top'
    lambda = lagrange_e_xx_tb
    variable = disp_x
    neighbor_var = disp_x
    component = 0
  [../]


[]

[BCs]
  # fix center point location
  [./centerfix_x]
    type = PresetBC
    boundary = 100
    variable = disp_x
    value = 0.0
  [../]
  [./centerfix_y]
    type = PresetBC
    boundary = 100
    variable = disp_y
    value = 0.0
  [../]


  ## fix side point x coordinate to inhibit rotation
  [./angularfix_1]
    type = PresetBC
    boundary = 101
    variable = disp_x
    value = 0.0
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
    petsc_options = '-snes_converged_reason'
    petsc_options_iname = '-ksp_gmres_restart -ksp_rtol -pc_type -pc_factor_shift_type'
    petsc_options_value = '150                1e-4       lu           nonzero'
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2 #does not seem to matter if bdf2 or implicit-euler
  solve_type = 'PJFNK'

  line_search = basic

  nl_rel_tol = 1.0e-8
  nl_abs_tol = 1.0e-10

  start_time = 0.0
  num_steps = 10

  l_max_its = 35
  nl_max_its = 12

  dtmax = 0.2
[]

[Outputs]
  execute_on = 'timestep_end'
  print_linear_residuals = true
  exodus = true
  file_base = out_test_lagrange_strain_test
  [./table]
    type = CSV
    delimiter = ' '
  [../]
[]
