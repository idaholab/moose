[Mesh]
  type = FileMesh
  file = split_blocks.e
  # NearestNodeLocator, which is needed by CoupledTiedValueConstraint,
  # only works with ReplicatedMesh currently
  parallel_type = replicated
[]

[Variables]
  [./u]
    block = left
  [../]
  [./v]
    block = right
  [../]
[]

[Kernels]
  [./diff_u]
    type = Diffusion
    variable = u
    block = left
  [../]
  [./diff_v]
    type = Diffusion
    variable = v
    block = right
  [../]
[]

[BCs]
  active = 'right left'
  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = v
    boundary = 4
    value = 1
  [../]
[]

[Constraints]
  [./value]
    type = CoupledTiedValueConstraint
    variable = u
    secondary = 2
    primary = 3
    primary_variable = v
  [../]
[]

[Preconditioning]
  active = 'SMP'
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  l_max_its = 100
  nl_max_its = 2
[]

[Outputs]
  file_base = out
  exodus = true
[]
