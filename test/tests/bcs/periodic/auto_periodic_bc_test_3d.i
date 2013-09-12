[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 10
  ny = 10
  nz = 10
  xmax = 40
  ymax = 40
  zmax = 40
  elem_type = HEX8
  ghosted_boundaries = '0 1 2 3 4 5' # Necessary for auto periodic bcs with ParallelMesh
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff forcing dot'

  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./forcing]
    type = GaussContForcing
    variable = u
    function = forcing_func
  [../]

  [./dot]
    type = TimeDerivative
    variable = u
  [../]
[]

[BCs]
  [./Periodic]
    [./all]
      variable = u
      auto_direction = 'x y z'
    [../]
  [../]
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 20
  solve_type = NEWTON
[]

[Output]
  file_base = out_auto_3d
  interval = 1
  exodus = true
  perf_log = true
[]

