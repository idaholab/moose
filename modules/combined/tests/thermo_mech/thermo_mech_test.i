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

[SolidMechanics]
  [./solid]
    disp_x = x_disp
    disp_y = y_disp
    disp_z = z_disp
  [../]
[]

[Kernels]
  [./heat]
    type = HeatConduction
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

  [./constant]
    type = LinearIsotropicMaterial
    block = 1

    disp_x = x_disp
    disp_y = y_disp
    disp_z = z_disp
    temp = temp

    youngs_modulus = 1.0
    poissons_ratio = .3
    thermal_expansion = 1e-5
  [../]

  [./heat1]
    type = HeatConductionMaterial
    block = 1

    specific_heat = 1.0
    thermal_conductivity = 1.0
  [../]

  [./density]
    type = Density
    block = 1
    density = 1.0
    disp_x = x_disp
    disp_y = y_disp
    disp_z = z_disp
  [../]

[]

[Executioner]
  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'



  nl_rel_tol = 1e-14

  l_tol = 1e-3
  l_max_its = 100

  dt = 1.0
  end_time = 1.0
[]

[Outputs]
  file_base = out
  exodus = true
[]
