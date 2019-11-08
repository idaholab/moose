[Mesh]
  [./gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
  [../]
[]

[Problem]
  gradient_scaling_vector = '1e3 1 1'
[]

[Variables]
  [./u]
    scaling = 1e-6
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./dt]
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

  dt = 1e-6
  num_steps = 2
[]

[Postprocessors]
  [./point]
    type = PointValue
    point = '0.25 0 0'
    variable = u
  [../]
[]

[Outputs]
  csv = true
[]
