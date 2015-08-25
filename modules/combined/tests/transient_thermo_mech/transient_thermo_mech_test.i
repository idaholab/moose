#Run with 4 procs

[Mesh]
  file = cube.e
[]

[Variables]
  [./x_disp]
    order = FIRST
    family = LAGRANGE
  [../]

  [./y_disp]
    order = FIRST
    family = LAGRANGE
  [../]

  [./z_disp]
    order = FIRST
    family = LAGRANGE
  [../]

  [./temp]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./solid_x]
    type = SolidMechX
    variable = x_disp
    y = y_disp
    z = z_disp
  [../]

  [./solid_y]
    type = SolidMechY
    variable = y_disp
    x = x_disp
    z = z_disp
  [../]

  [./solid_z]
    type = SolidMechZ
    variable = z_disp
    x = x_disp
    y = y_disp
  [../]

  [./solid_x_ie]
    type = SolidMechImplicitEuler
    variable = x_disp
  [../]

  [./solid_y_ie]
    type = SolidMechImplicitEuler
    variable = y_disp
  [../]

  [./solid_z_ie]
    type = SolidMechImplicitEuler
    variable = z_disp
  [../]

  [./solid_couple_x]
    type = SolidMechTempCoupleX
    variable = x_disp
    temp = temp
  [../]

  [./solid_couple_y]
    type = SolidMechTempCoupleY
    variable = y_disp
    temp = temp
  [../]

  [./solid_couple_z]
    type = SolidMechTempCoupleZ
    variable = z_disp
    temp = temp
  [../]

  [./heat]
    type = HeatConduction
    variable = temp
  [../]

  [./heat_ie]
    type = HeatConductionTimeDerivative
    variable = temp
  [../]
[]

[BCs]
  [./bottom_x]
    type = DirichletBC
    variable = x_disp
    boundary = 1
    value = 0.0
  [../]

  [./bottom_y]
    type = DirichletBC
    variable = y_disp
    boundary = 1
    value = 0.0
  [../]

  [./bottom_z]
    type = DirichletBC
    variable = z_disp
    boundary = 1
    value = 0.0
  [../]

  [./bottom_temp]
    type = DirichletBC
    variable = temp
    boundary = 1
    value = 10.0
  [../]
[]

[Materials]

  [./constantHeat]
    type = HeatConductionMaterial
    block = 1
    thermal_conductivity = 1.0
    specific_heat = 1.0
  [../]
  [./constantElastic]
    type = LinearElasticityMaterial
    block = 1
    temp = temp
    youngs_modulus = 1
    poissons_ratio = .3
    thermal_expansion = 1e-5
    t_ref = 0
  [../]
  [./density]
    type = Density
    block = 1
    density = 1.0
  [../]
[]

[Executioner]
  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'



  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'

  nl_rel_tol = 1e-14

  start_time = 0.0
  num_steps = 2
  dt = 1
[]

[Outputs]
  file_base = out
  exodus = true
[]
