[Mesh]#Comment
  file = cube.e
#  displacements = 'disp_x disp_y disp_z'
[] # Mesh

[Variables]

  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]

  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]

  [./disp_z]
    order = FIRST
    family = LAGRANGE
  [../]

[] # Variables

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
  [../]
[]

[BCs]

  [./2_x]
    type = DirichletBC
    variable = disp_x
    boundary = 2
    value = 0.0
  [../]
  [./2_y]
    type = DirichletBC
    variable = disp_y
    boundary = 2
    value = 0.0
  [../]
  [./2_z]
    type = DirichletBC
    variable = disp_z
    boundary = 2
    value = 0.0
  [../]

[] # BCs

[Contact]
  [./fred]
    master = 1
    slave = 2
    disp_x = disp_x
    disp_y = disp_y
    system = Constraint
  [../]
[]

[Materials]

  [./goo]
    type = Elastic
    block = 1

    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z

    youngs_modulus = 1e6
    poissons_ratio = 0.0

  [../]

[] # Materials

[Executioner]

  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'




  nl_abs_tol = 1e-10

  l_max_its = 20

  start_time = 0.0
  dt = 1.0
  num_steps = 2
  end_time = 2.0
[] # Executioner

[Outputs]
  file_base = out
  exodus = true
[] # Outputs
