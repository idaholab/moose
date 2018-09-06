[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 8
  ny = 8

  # we test distributed
  parallel_type = distributed
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff_u]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left_u]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]

  [./right_u]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 10
  [../]
[]


[AuxVariables]
  [./ghosted_values]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./proc]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./ghosted_values]
    type = GhostAux
    variable = ghosted_values
    ghost_user_object = ghost_uo
    execute_on = timestep_end

  [../]
  [./proc ]
    type = ProcessorIDAux
    variable = proc
    execute_on = initial
  [../]
[]

[UserObjects]
  [./ghost_uo]
    type = GhostUserObject
    execute_on = timestep_end
    variable = u
    element_side_neighbor_layers = 1
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
