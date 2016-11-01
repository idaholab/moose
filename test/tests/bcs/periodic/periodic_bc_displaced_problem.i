[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 50
  ny = 50
  nz = 0
  xmax = 40
  ymax = 40
  zmax = 0
  elem_type = QUAD4
[]

[GlobalParams]
  use_displaced_mesh = false
  displacements = 'disp_x disp_y'
[]


[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]

  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]

  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./forcing]
    type = GaussContForcing
    variable = u
  [../]

  [./dot]
    type = TimeDerivative
    variable = u
  [../]

  [./diff_x]
    type = Diffusion
    variable = disp_x
  [../]

  [./diff_y]
    type = Diffusion
    variable = disp_y
  [../]

[]

[BCs]
  [./Periodic]
    [./x]
      variable = u
      primary = 3
      secondary = 1
      translation = '40 0 0'
    [../]

    [./y]
      variable = u
      primary = 0
      secondary = 2
      translation = '0 40 0'
    [../]
  [../]

  [./disp_0]
    type = DirichletBC
    variable = disp_x
    boundary = '0'
    value = 0.01
  [../]

  [./disp_1]
    type = DirichletBC
    variable = disp_y
    boundary = '0'
    value = 0.01
  [../]
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 20
  solve_type = NEWTON
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = out_displaced_problem
  exodus = true
[]
