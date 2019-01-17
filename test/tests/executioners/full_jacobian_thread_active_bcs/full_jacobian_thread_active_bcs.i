[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 5
[]

[Variables]
  [./u]
  [../]
[]

[ICs]
  [./ic]
    type = ConstantIC
    variable = u
    value = 1
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = RobinBC
    variable = u
    boundary = left
    enable = false
  [../]
  [./right]
    type = RobinBC
    variable = u
    boundary = right
  [../]
[]

[Preconditioning]
  [./pc]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  nl_max_its = 1
[]
