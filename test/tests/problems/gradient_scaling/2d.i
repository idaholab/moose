[Mesh]
  [./gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 20
    xmax = 0.5
    ymax = 1.25
  [../]
[]

[Variables]
  [./u]
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
  [./force]
    type = BodyForce
    variable = u
    value = 1e4
  [../]
[]

[BCs]
  [./zero]
    type = DirichletBC
    variable = u
    boundary = 'left bottom top'
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

  dt = 1e-3
  num_steps = 2
[]

[Postprocessors]
  [./point]
    type = PointValue
    point = '0.125 0.5 0'
    variable = u
  [../]
[]

[Outputs]
  csv = true
[]
