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

  [./body_force]
    type = BodyForce
    variable = y_disp
    value = 1e10
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
[]

[Materials]

  [./constant]
    type = LinearElasticityMaterial
    block = 1
    youngs_modulus = 2e11
    poissons_ratio = .3
  [../]
[]

[Executioner]
  type = Steady

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'



[]

[Outputs]
  file_base = out
  exodus = true
[]
