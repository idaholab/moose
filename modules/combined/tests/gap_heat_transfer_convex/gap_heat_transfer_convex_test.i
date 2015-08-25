[Mesh]
  file = gap_heat_transfer_convex_test.e
  displacements = 'displ_x displ_y displ_z'
[]

[Functions]

  [./disp]
    type = PiecewiseLinear
    x = '0 2.0'
    y = '0 1.0'
  [../]

  [./temp]
    type = PiecewiseLinear
    x = '0     1'
    y = '200 200'
  [../]
[]

[Variables]
  [./displ_x]
    order = FIRST
    family = LAGRANGE
  [../]

  [./displ_y]
    order = FIRST
    family = LAGRANGE
  [../]

  [./displ_z]
    order = FIRST
    family = LAGRANGE
  [../]

  [./temp]
    order = FIRST
    family = LAGRANGE
    initial_condition = 100
  [../]
[]

[ThermalContact]
  [./thermal_contact]
    type = GapHeatTransfer
    variable = temp
    master = 2
    slave = 3
  [../]
[]

[SolidMechanics]
  [./solid]
    disp_x = displ_x
    disp_y = displ_y
    disp_z = displ_z
  [../]
[]


[Kernels]

  [./heat]
    type = HeatConduction
    variable = temp
  [../]
[]


[BCs]

  [./move_right]
    type = FunctionDirichletBC
    boundary = '3'
    variable = displ_x
    function = disp
  [../]

  [./fixed_x]
    type = DirichletBC
    boundary = '1'
    variable = displ_x
    value = 0
  [../]

  [./fixed_y]
    type = DirichletBC
    boundary = '1 2 3 4'
    variable = displ_y
    value = 0
  [../]

  [./fixed_z]
    type = DirichletBC
    boundary = '1 2 3 4'
    variable = displ_z
    value = 0
  [../]

  [./temp_bottom]
    type = FunctionDirichletBC
    boundary = 1
    variable = temp
    function = temp
  [../]

  [./temp_top]
    type = DirichletBC
    boundary = 4
    variable = temp
    value = 100
  [../]
[]

[Materials]

  [./dummy]
    type = Elastic
    block = '1 2'

    disp_x = displ_x
    disp_y = displ_y
    disp_z = displ_z

    youngs_modulus = 1e6
    poissons_ratio = .3

    temp = temp
    thermal_expansion = 0
  [../]

  [./heat1]
    type = HeatConductionMaterial
    block = 1

    specific_heat = 1.0
    thermal_conductivity = 1.0
  [../]

  [./heat2]
    type = HeatConductionMaterial
    block = 2

    specific_heat = 1.0
    thermal_conductivity = 1.0
  [../]

  [./density]
    type = Density
    block = '1 2'
    density = 1.0
    disp_x = displ_x
    disp_y = displ_y
    disp_z = displ_z
  [../]

[]

[Executioner]
  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'




  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre    boomeramg      4'


  line_search = 'none'


  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10

  l_tol = 1e-3
  l_max_its = 100

  start_time = 0.0
  dt = 1e-1
  end_time = 2.0
[]

[Outputs]
  file_base = out
  exodus = true
[]
