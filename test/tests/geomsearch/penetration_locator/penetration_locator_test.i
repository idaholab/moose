[Mesh]
  file = 2dcontact_collide.e
  # This test will not work in parallel with ParallelMesh enabled
  # due to a bug in the GeometricSearch system.
  distribution = serial
  # uniform_refine = 1
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./penetration]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff'

  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[AuxBCs]
  active = 'penetrate'

  [./penetrate]
    type = PenetrationAux
    variable = penetration
    boundary = 2
    paired_boundary = 3
  [../]
[]

[BCs]
  active = 'block1_left block1_right block2_left block2_right'

  [./block1_left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]

  [./block1_right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]

  [./block2_left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  [../]

  [./block2_right]
    type = DirichletBC
    variable = u
    boundary = 4
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
  perf_log = true
[]


