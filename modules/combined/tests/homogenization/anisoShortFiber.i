#
# Test from:
#   Multiple Scale Analysis of Heterogeneous Elastic Structures Using
#   Homogenization Theory and Voronoi Cell Finite Element Method
#   by S.Ghosh et. al, Int J. Solids Structures, Vol. 32, No. 1,
#   pp. 27-62, 1995.
#
# From that paper, elastic constants should be:
# E1111: 122.4
# E2222: 151.2
# E1212:  42.1
# E1122:  36.23
#

[Mesh]
  file = anisoShortFiber.e
  # To calculate matching values, refine the mesh one time.
  # We use a coarse mesh for speed in this test.
  # uniform_refine = 1
[]

[Variables]
 [./dx_xx]
    order = FIRST
    family = LAGRANGE
 [../]

 [./dy_xx]
 order = FIRST
 family = LAGRANGE
 [../]

 [./dx_yy]
 order = FIRST
 family = LAGRANGE
 [../]

 [./dy_yy]
 order = FIRST
 family = LAGRANGE
 [../]

 [./dx_xy]
 order = FIRST
 family = LAGRANGE
 [../]

 [./dy_xy]
 order = FIRST
 family = LAGRANGE
 [../]

[]


[Kernels]
 [./homo_dx_xx]
    type = HomogenizationKernel
    variable = dx_xx
    component = 0
    column = 0
    appended_property_name = xx
 [../]

 [./homo_dy_xx]
 type = HomogenizationKernel
 variable = dy_xx
 component = 1
 column = 0
 appended_property_name = xx
 [../]

 [./homo_dx_yy]
 type = HomogenizationKernel
 variable = dx_yy
 component = 0
 column = 1
 appended_property_name = yy
 [../]

 [./homo_dy_yy]
 type = HomogenizationKernel
 variable = dy_yy
 component = 1
 column = 1
 appended_property_name = yy
 [../]

 [./homo_dx_xy]
 type = HomogenizationKernel
 variable = dx_xy
 component = 0
 column = 3
 appended_property_name = xy
 [../]

 [./homo_dy_xy]
 type = HomogenizationKernel
 variable = dy_xy
 component = 1
 column = 3
 appended_property_name = xy
 [../]

[]

[SolidMechanics]
  [./solid_xx]
   disp_x = dx_xx
   disp_y = dy_xx
   appended_property_name = xx
  [../]

 [./solid_yy]
  disp_x = dx_yy
  disp_y = dy_yy
  appended_property_name = yy
 [../]

 [./solid_xy]
  disp_x = dx_xy
  disp_y = dy_xy
  appended_property_name = xy
 [../]
[]



[BCs]


  [./Periodic]
  [./top_bottom]
    primary = 30
    secondary = 40
    translation = '0 1 0'
  [../]

  [./left_right]
    primary = 10
    secondary = 20
    translation = '1 0 0'
  [../]
  [../]

  [./dx_xx_anchor]
    type = DirichletBC
    variable = dx_xx
    boundary = 1
    value = 0.0
  [../]
  [./dy_xx_anchor]
    type = DirichletBC
    variable = dy_xx
    boundary = 1
    value = 0.0
  [../]

  [./dx_yy_anchor]
    type = DirichletBC
    variable = dx_yy
    boundary = 1
    value = 0.0
  [../]
  [./dy_yy_anchor]
    type = DirichletBC
    variable = dy_yy
    boundary = 1
    value = 0.0
  [../]

  [./dx_xy_anchor]
    type = DirichletBC
    variable = dx_xy
    boundary = 1
    value = 0.0
  [../]
  [./dy_xy_anchor]
    type = DirichletBC
    variable = dy_xy
    boundary = 1
    value = 0.0
  [../]

[]

[Materials]


 [./block1]
 type =  LinearAnisotropicMaterial
 block = 1
 material_constant_c11 = 81.360117
 material_constant_c12 = 26.848839
 material_constant_c44 = 27.255639
 euler_angle_1 = 0.0
 euler_angle_2 = 0.0
 euler_angle_3 = 0.0

 disp_x = dx_xx
 disp_y = dy_xx


 youngs_modulus = 72.5
 poissons_ratio = 0.33
 appended_property_name = xx
 [../]

 [./block2]
 type =  LinearAnisotropicMaterial
 block = 1
 material_constant_c11 = 81.360117
 material_constant_c12 = 26.848839
 material_constant_c44 = 27.255639
 euler_angle_1 = 0.0
 euler_angle_2 = 0.0
 euler_angle_3 = 0.0

 disp_x = dx_yy
 disp_y = dy_yy


 youngs_modulus = 72.5
 poissons_ratio = 0.33
 appended_property_name = yy
 [../]

 [./block3]
 type =  LinearAnisotropicMaterial
 block = 1
 material_constant_c11 = 81.360117
 material_constant_c12 = 26.848839
 material_constant_c44 = 27.255639
 euler_angle_1 = 0.0
 euler_angle_2 = 0.0
 euler_angle_3 = 0.0

 disp_x = dx_xy
 disp_y = dy_xy


 youngs_modulus = 72.5
 poissons_ratio = 0.33
 appended_property_name = xy
 [../]

 [./block4]
 type =  LinearAnisotropicMaterial
 block = 2
 material_constant_c11 = 416.66667
 material_constant_c12 = 83.333333
 material_constant_c44 = 166.66667
 euler_angle_1 = 0.0
 euler_angle_2 = 0.0
 euler_angle_3 = 0.0

 disp_x = dx_xx
 disp_y = dy_xx


 youngs_modulus = 400
 poissons_ratio = 0.2
 appended_property_name = xx
 [../]

 [./block5]
 type =  LinearAnisotropicMaterial
 block = 2
 material_constant_c11 = 416.66667
 material_constant_c12 = 83.333333
 material_constant_c44 = 166.66667
 euler_angle_1 = 0.0
 euler_angle_2 = 0.0
 euler_angle_3 = 0.0

 disp_x = dx_yy
 disp_y = dy_yy


 youngs_modulus = 400
 poissons_ratio = 0.2
 appended_property_name = yy
 [../]

 [./block6]
 type =  LinearAnisotropicMaterial
 block = 2
 material_constant_c11 = 416.66667
 material_constant_c12 = 83.333333
 material_constant_c44 = 166.66667
 euler_angle_1 = 0.0
 euler_angle_2 = 0.0
 euler_angle_3 = 0.0

 disp_x = dx_xy
 disp_y = dy_xy


 youngs_modulus = 400
 poissons_ratio = 0.2
 appended_property_name = xy
 [../]

[]

[Postprocessors]

  [./E1111]
    type = HomogenizedElasticConstants
    variable = dx_xx
    appended_property_name = xx
    row = 0
    column = 0
    dx_xx = dx_xx
    dy_xx = dy_xx
    dx_yy = dx_yy
    dy_yy = dy_yy
    dx_xy = dx_xy
    dy_xy = dy_xy
    execute_on = 'initial timestep_end'
  [../]

  [./E2222]
    type = HomogenizedElasticConstants
    variable = dx_xx
    appended_property_name = xx
    row = 1
    column = 1
    dx_xx = dx_xx
    dy_xx = dy_xx
    dx_yy = dx_yy
    dy_yy = dy_yy
    dx_xy = dx_xy
    dy_xy = dy_xy
    execute_on = 'initial timestep_end'
  [../]

  [./E1122]
    type = HomogenizedElasticConstants
    variable = dx_xx
    appended_property_name = xx
    row = 0
    column = 1
    dx_xx = dx_xx
    dy_xx = dy_xx
    dx_yy = dx_yy
    dy_yy = dy_yy
    dx_xy = dx_xy
    dy_xy = dy_xy
    execute_on = 'initial timestep_end'
  [../]


  [./E2211]
    type = HomogenizedElasticConstants
    variable = dx_xx
    appended_property_name = xx
    row = 1
    column = 0
    dx_xx = dx_xx
    dy_xx = dy_xx
    dx_yy = dx_yy
    dy_yy = dy_yy
    dx_xy = dx_xy
    dy_xy = dy_xy
    execute_on = 'initial timestep_end'
  [../]

  [./E1212]
    type = HomogenizedElasticConstants
    variable = dx_xx
    appended_property_name = xx
    row = 3
    column = 3
    dx_xx = dx_xx
    dy_xx = dy_xx
    dx_yy = dx_yy
    dy_yy = dy_yy
    dx_xy = dx_xy
    dy_xy = dy_xy
    execute_on = 'initial timestep_end'
  [../]

[]

[Preconditioning]

 [./SMP]
  type = SMP
   full = true
 [../]

[]


[Executioner]

 type = Transient


  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'


 petsc_options = '-ksp_gmres_modifiedgramschmidt'
 petsc_options_iname = '-ksp_gmres_restart -pc_type   -pc_hypre_type -pc_hypre_boomeramg_max_iter -pc_hypre_boomeramg_grid_sweeps_all -ksp_type -mat_mffd_type'
 petsc_options_value = '201                 hypre       boomeramg      2                            2                                   fgmres    ds'


  line_search = 'none'


  l_tol = 1e-4
  l_max_its = 40
  nl_max_its = 40
  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-10

  start_time = 0.0
  end_time = 10.0
  num_steps = 1
  dt = 10
[]


[Outputs]
  exodus = true
  csv = true
[]
