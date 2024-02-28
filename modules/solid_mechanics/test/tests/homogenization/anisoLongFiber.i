#
# Test from:
#   Multiple Scale Analysis of Heterogeneous Elastic Structures Using
#   Homogenization Theory and Voronoi Cell Finite Element Method
#   by S.Ghosh et. al, Int J. Solids Structures, Vol. 32, No. 1,
#   pp. 27-62, 1995.
#
# From that paper, elastic constants should be:
# E1111: 136.1
# E2222: 245.8
# E1212:  46.85
# E1122:  36.08
#
# Note: this is for plane stress conditions
#

[Mesh]
  file = anisoLongFiber.e
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

  [./div_x_xx]
    type = StressDivergenceTensors
    component = 0
    variable = dx_xx
    displacements = 'dx_xx dy_xx'
    use_displaced_mesh = false
    base_name = xx
  [../]
  [./div_y_xx]
    type = StressDivergenceTensors
    component = 1
    variable = dy_xx
    displacements = 'dx_xx dy_xx'
    use_displaced_mesh = false
    base_name = xx
  [../]
  [./div_x_yy]
    type = StressDivergenceTensors
    component = 0
    variable = dx_yy
    displacements = 'dx_yy dy_yy'
    use_displaced_mesh = false
    base_name = yy
  [../]
  [./div_y_yy]
    type = StressDivergenceTensors
    component = 1
    variable = dy_yy
    displacements = 'dx_yy dy_yy'
    use_displaced_mesh = false
    base_name = yy
  [../]
  [./div_x_xy]
    type = StressDivergenceTensors
    component = 0
    variable = dx_xy
    displacements = 'dx_xy dy_xy'
    use_displaced_mesh = false
    base_name = xy
  [../]
  [./div_y_xy]
    type = StressDivergenceTensors
    component = 1
    variable = dy_xy
    displacements = 'dx_xy dy_xy'
    use_displaced_mesh = false
    base_name = xy
  [../]

  [./homo_dx_xx]
    type = AsymptoticExpansionHomogenizationKernel
    variable = dx_xx
    component = 0
    column = xx
    base_name = xx
  [../]
  [./homo_dy_xx]
    type = AsymptoticExpansionHomogenizationKernel
    variable = dy_xx
    component = 1
    column = xx
    base_name = xx
  [../]

  [./homo_dx_yy]
    type = AsymptoticExpansionHomogenizationKernel
    variable = dx_yy
    component = 0
    column = yy
    base_name = yy
  [../]
  [./homo_dy_yy]
    type = AsymptoticExpansionHomogenizationKernel
    variable = dy_yy
    component = 1
    column = yy
    base_name = yy
  [../]

  [./homo_dx_xy]
    type = AsymptoticExpansionHomogenizationKernel
    variable = dx_xy
    component = 0
    column = xy
    base_name = xy
  [../]
  [./homo_dy_xy]
    type = AsymptoticExpansionHomogenizationKernel
    variable = dy_xy
    component = 1
    column = xy
    base_name = xy
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

  [./elastic_stress_xx]
    type = ComputeLinearElasticStress
    base_name = xx
  [../]
  [./elastic_stress_yy]
    type = ComputeLinearElasticStress
    base_name = yy
  [../]
  [./elastic_stress_xy]
    type = ComputeLinearElasticStress
    base_name = xy
  [../]
  [./strain_xx]
    type = ComputeSmallStrain
    displacements = 'dx_xx dy_xx'
    base_name = xx
  [../]
  [./strain_yy]
    type = ComputeSmallStrain
    displacements = 'dx_yy dy_yy'
    base_name = yy
  [../]
  [./strain_xy]
    type = ComputeSmallStrain
    displacements = 'dx_xy dy_xy'
    base_name = xy
  [../]


  [./block1]
    type =  ComputeElasticityTensor
    block = 1
    fill_method = symmetric9
    C_ijkl = '81.360117 26.848839 26.848839 81.360117 26.848839 81.360117 27.255639 27.255639 27.255639'
    base_name = xx
  [../]

  [./block2]
    type =  ComputeElasticityTensor
    block = 1
    fill_method = symmetric9
    C_ijkl = '81.360117 26.848839 26.848839 81.360117 26.848839 81.360117 27.255639 27.255639 27.255639'
    base_name = yy
  [../]

  [./block3]
    type =  ComputeElasticityTensor
    block = 1
    fill_method = symmetric9
    C_ijkl = '81.360117 26.848839 26.848839 81.360117 26.848839 81.360117 27.255639 27.255639 27.255639'
    base_name = xy
  [../]

  [./block4]
    type =  ComputeElasticityTensor
    block = 2
    fill_method = symmetric9
    C_ijkl = '416.66667 83.33333 83.33333 416.6667 83.33333 416.66667 166.66667 166.66667 166.66667'
    base_name = xx
  [../]

  [./block5]
    type =  ComputeElasticityTensor
    block = 2
    fill_method = symmetric9
    C_ijkl = '416.66667 83.33333 83.33333 416.6667 83.33333 416.66667 166.66667 166.66667 166.66667'
    base_name = yy
  [../]

  [./block6]
    type =  ComputeElasticityTensor
    block = 2
    fill_method = symmetric9
    C_ijkl = '416.66667 83.33333 83.33333 416.6667 83.33333 416.66667 166.66667 166.66667 166.66667'
    base_name = xy
 [../]

[]

[Postprocessors]

  [./E1111]
    type = AsymptoticExpansionHomogenizationElasticConstants
    base_name = xx
    row = xx
    column = xx
    dx_xx = dx_xx
    dy_xx = dy_xx
    dx_yy = dx_yy
    dy_yy = dy_yy
    dx_xy = dx_xy
    dy_xy = dy_xy
    execute_on = 'initial timestep_end'
  [../]

  [./E2222]
    type = AsymptoticExpansionHomogenizationElasticConstants
    base_name = xx
    row = yy
    column = yy
    dx_xx = dx_xx
    dy_xx = dy_xx
    dx_yy = dx_yy
    dy_yy = dy_yy
    dx_xy = dx_xy
    dy_xy = dy_xy
    execute_on = 'initial timestep_end'
  [../]

  [./E1122]
    type = AsymptoticExpansionHomogenizationElasticConstants
    base_name = xx
    row = xx
    column = yy
    dx_xx = dx_xx
    dy_xx = dy_xx
    dx_yy = dx_yy
    dy_yy = dy_yy
    dx_xy = dx_xy
    dy_xy = dy_xy
    execute_on = 'initial timestep_end'
  [../]


  [./E2211]
    type = AsymptoticExpansionHomogenizationElasticConstants
    base_name = xx
    row = yy
    column = xx
    dx_xx = dx_xx
    dy_xx = dy_xx
    dx_yy = dx_yy
    dy_yy = dy_yy
    dx_xy = dx_xy
    dy_xy = dy_xy
    execute_on = 'initial timestep_end'
  [../]


  [./E1212]
    type = AsymptoticExpansionHomogenizationElasticConstants
    base_name = xx
    row = xy
    column = xy
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
[]
