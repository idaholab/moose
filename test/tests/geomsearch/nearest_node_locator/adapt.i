[Mesh]
  file = 2dcontact_collide.e
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./distance]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./conv]
    type = Convection
    variable = u
    velocity = '1 0 0'
  [../]
[]

[AuxKernels]
  [./zero]
    type = ConstantAux
    variable = distance
  [../]
  [./distance]
    type = NearestNodeDistanceAux
    variable = distance
    boundary = 2
    paired_boundary = 3
  [../]
[]

[BCs]
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
  type = Transient
  num_steps = 3
  dt = 1

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Adaptivity]
  marker = uniform
  [./Markers]
    [./uniform]
      type = UniformMarker
      mark = refine
    [../]
  [../]
[]

[Outputs]
  exodus = true
[]
