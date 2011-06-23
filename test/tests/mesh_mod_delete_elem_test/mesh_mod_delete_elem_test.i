[Mesh]
  file = pellets.e
  active = 'Modifier'

  [./Generation]
    dim = 3
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1  
    zmin = 0
    zmax = 0.1  
    nx = 4
    ny = 4
    nz = 2
  [../]

  [./Modifier]
    [./deleter]
      type = ElementDeleter
      function = mesh_damage_sphere
    [../]
  [../]
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Functions]
  [./mesh_damage_sphere]
    type = SphereFunction
    x_center = 0.0043
    y_center = 0.0
    z_center = 0.025
    radius = 0.0012
  [../]
[]

[Kernels]
  active = 'diff'

  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  active = ' '

  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
[]

[Executioner]
  type = Steady
  petsc_options = '-snes_mf_operator'
[]

[Output]
  file_base = out
  output_initial = true
  interval = 1
  exodus = true
  print_linear_residuals = true
  perf_log = true
[]
