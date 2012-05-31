[Mesh]
  file = 3-4-torus.e
[]

[Variables]
  active = 'diffused'

  [./diffused]
    order = FIRST
    family = LAGRANGE
  [../]
[]
[Kernels]
  active = 'diff'

  [./diff]
    type = Diffusion
    variable = diffused
  [../]
[]

[DiracKernels]
  active = 'example_point_source'

  [./example_point_source]
    type = ExampleDirac
    variable = diffused
    value = 1.0
    point = '-2.1 -5.08 0.7'
  [../]
[]

[BCs]
  active = 'left right'

  [./right]
    type = DirichletBC
    variable = diffused
    boundary = 'right'
    value = 0
  [../]

  [./left]
    type = DirichletBC
    variable = diffused
    boundary = 'left'
    value = 1
  [../]
[]

# The Preconditioning block
[Preconditioning]
  active = 'pbp'

  [./pbp]
    type = PBP
    solve_order = 'diffused'
    preconditioner  = 'AMG'
  [../]
[]

[Executioner]
  type = Steady
  petsc_options = '-snes_mf'
[]

[Output]
  file_base = out
  interval = 1
  exodus = true
  perf_log = true
[]
