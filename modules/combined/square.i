[GlobalParams]
  order = FIRST
  family = LAGRANGE
[]

[Problem]
  type = ReferenceResidualProblem
  solution_variables = 'disp_x disp_y temp'
  reference_residual_variables = 'saved_x saved_y saved_t'
#                x0   y0   x1   y1   t0    t1
  XFEM_cuts = ' 3853  -1860  3603  -1810   0   2 
                3603  -1810  3293  -1610   2   4  
                3293  -1610  2953  -1310   4   6  
                2953  -1310  2733  -1090   6   8  
                2733  -1090  2533   -920   8  10  
                2533   -920  2419   -568  10  12  
                2419   -568  2199   -228  12  14  
                2199   -228  1919    -28  14  16  
                1919    -28  1589     12  16  18  
                1589     12  1329    -48  18  20  
                1329    -48  1209   -248  20  22  
                1209   -248  1199   -388  22  24  
                1199   -388  1209   -538  24  26  
                1209   -538  1159   -628  26  28  
                1159   -628  1019   -688  28  30  
                1019   -688   809   -768  30  32  
                 809   -768   719   -858  32  34  
                 719   -858   629   -928  34  36  
                 629   -928   489   -968  36  38  
                 489   -968   169  -1158  38  40  
                 169  -1158     9  -1418  40  42  
                   9  -1418   -11  -1688  42  44  
                3849  -6088  3339  -6178   0   2
                3339  -6178  2889  -6238   2   4  
                2889  -6238  2409  -6238   4   6  
                2409  -6238  1899  -6078   6   8  
                1899  -6078  1389  -5878   8  10  
                1389  -5878   999  -5648  10  12  
                 999  -5648   579  -5248  12  14  
                 579  -5248   289  -4778  14  16  
                 289  -4778    59  -4408  16  18  
                  59  -4408     9  -4158  18  20  
                   9  -4158    19  -3848  20  22  
                  19  -3848   109  -3468  22  24  
                 109  -3468   189  -3138  24  26  
                 189  -3138   169  -2868  26  28  
                 169  -2868    89  -2658  28  30  
                  89  -2658    43  -2477  30  32  
                  43  -2477    65  -2251  32  34  
                  65  -2251    99  -2108  34  36  
                  99  -2108    99  -1928  36  38  
                  99  -1928   -11  -1688  38  40
                2513  -5830  2593  -5430   0   2
                2593  -5430  2633  -5090   2   4
                2633  -5090  2623  -4940   4   6
                2623  -4940  2563  -4810   6   8
                2563  -4810  2383  -4740   8  10
                2383  -4740  2263  -4590  10  12
                2263  -4590  2273  -4350  12  14
                2273  -4350  2413  -4160  14  16
                2413  -4160  2713  -4120  16  18
                2713  -4120  2993  -4280  18  20
                2993  -4280  3133  -4590  20  22
                3133  -4590  3123  -4880  22  24
                3123  -4880  3003  -5150  24  26
                1979  -3408  1839  -3478  26  28
                1839  -3478  1649  -3408  28  30
                1649  -3408  1579  -3268  30  32'
# Nostril breaks it:
#                '543  -1230   653  -1340  26  28
#                 653  -1340   683  -1490  28  30
#                 683  -1490   593  -1560  30  32'

  XFEM_cut_translate = '1200 -300'
  XFEM_cut_scale = '1.4e-4 -1.4e-4'
[]

[Mesh]
  file = square.e
  displacements = 'disp_x disp_y'
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./temp]
  [../]
[]

[AuxVariables]
  [./xfem_volfrac]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./xfem_cut_origin_x]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./xfem_cut_origin_y]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./xfem_cut_origin_z]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./xfem_cut_normal_x]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./xfem_cut_normal_y]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./xfem_cut_normal_z]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./saved_x]
  [../]
  [./saved_y]
  [../]
  [./saved_z]
  [../]
  [./saved_t]
  [../]
[]

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
    save_in_disp_x = saved_x
    save_in_disp_y = saved_y
    xfem_volfrac = xfem_volfrac
    temp = temp
  [../]
[]

[Kernels]
  [./heat]
    type = HeatConduction
    variable = temp
    xfem_volfrac = xfem_volfrac
    save_in = saved_t
  [../]
[]

[AuxKernels]
  [./xfem_volfrac]
    type = XFEMVolFracAux
    variable = xfem_volfrac
    execute_on = timestep_begin
  [../]
  [./xfem_cut_origin_x]
    type = XFEMCutPlaneAux
    variable = xfem_cut_origin_x
    quantity = origin_x
    execute_on = timestep
  [../]
  [./xfem_cut_origin_y]
    type = XFEMCutPlaneAux
    variable = xfem_cut_origin_y
    quantity = origin_y
    execute_on = timestep
  [../]
  [./xfem_cut_origin_z]
    type = XFEMCutPlaneAux
    variable = xfem_cut_origin_z
    quantity = origin_z
    execute_on = timestep
  [../]
  [./xfem_cut_normal_x]
    type = XFEMCutPlaneAux
    variable = xfem_cut_normal_x
    quantity = normal_x
    execute_on = timestep
  [../]
  [./xfem_cut_normal_y]
    type = XFEMCutPlaneAux
    variable = xfem_cut_normal_y
    quantity = normal_y
    execute_on = timestep
  [../]
  [./xfem_cut_normal_z]
    type = XFEMCutPlaneAux
    variable = xfem_cut_normal_z
    quantity = normal_z
    execute_on = timestep
  [../]
[]

[Functions]
  [./pull]
    type = PiecewiseLinear
    x='0     1  100'
    y='0  0.01 0.01'
  [../]
[]

[BCs]
  [./bottomx]
    type = PresetBC
    boundary = 1
    variable = disp_x
    value = 0.0
  [../]
  [./bottomy]
    type = PresetBC
    boundary = 1
    variable = disp_y
    value = 0.0
  [../]
  [./topx]
    type = PresetBC
    boundary = 3
    variable = disp_x
    value = 0.0
  [../]
  [./topy]
    type = FunctionPresetBC
    boundary = 3
    variable = disp_y
    function = pull
  [../]
  [./bottomt]
    type = PresetBC
    boundary = 2
    variable = temp
    value = 0.0
  [../]
  [./topt]
    type = PresetBC
    boundary = 4
    variable = temp
    value = 1.0
  [../]
[]

[Materials]
  [./linelast]
    type = LinearIsotropicMaterial
    block = 1
    disp_x = disp_x
    disp_y = disp_y
    poissons_ratio = 0.3
    youngs_modulus = 1e6
    thermal_expansion = 0.02
    t_ref = 0.5
    temp = temp
  [../]
  [./heatcond]
    type = HeatConductionMaterial
    block = 1
    thermal_conductivity = 3.0
    specific_heat = 300.0
  [../]
  [./density]
    type = Density
    block = 1
    density = 1.0
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'
  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre    boomeramg      8'

  line_search = 'none'

  [./Predictor]
    type = SimplePredictor
    scale = 1.0
  [../]

# controls for linear iterations
  l_max_its = 100
  l_tol = 1e-2

# controls for nonlinear iterations
  nl_max_its = 15
  nl_rel_tol = 1e-4
  nl_abs_tol = 1e-10

# time control
  start_time = 0.0
  dt = 1.0
  end_time = 50.0
  num_steps = 5000
[]

[Outputs]
  file_base = square_out
##  interval = 1
  output_initial = true
  exodus = true
  [./console]
    type = Console
    perf_log = true
    linear_residuals = true
  [../]
[]
