[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 5
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./from_parent]
  [../]
  [./elemental_from_parent]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./td]
    type = TimeDerivative
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 2
  dt = 0.01
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
  #
[]

