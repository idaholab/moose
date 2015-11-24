[Variables]
  [./convected]
    order = FIRST
    family = LAGRANGE
  [../]
[]
[BCs]
  [./bottom]
    type = DirichletBC
    variable = convected
    boundary = 'bottom'
    value = 1
  [../]
  [./top]
    type = DirichletBC
    variable = convected
    boundary = 'top'
    value = 0
  [../]
[]
[Executioner]
  dt = 1
  num_steps = 2
  type = Transient
[]
[Kernels]
  [./diff]
    type = Diffusion
    variable = convected
  [../]
  [./conv]
    type = Convection
    variable = convected
    velocity = '0.0 0.0 1.0'
  [../]
  [./TimeDerivative]
    type = TimeDerivative
    variable = convected
  [../]
[]
[Mesh]
  type = FileMesh
  file = mug.e
#dim = 3
#  nx = 10
#  ny = 10
[]
[Outputs]
  file_base = out
  [./ICEUpdater]
    item_id = 1
    type = ICEUpdater
    url = http://localhost:8081/ice/update
    networkingTool = 'curl'
  [../]
[]
[Postprocessors]
  [./NodalVariableValue10]
    nodeid = 10
    type = NodalVariableValue
    variable = convected
  [../]
[]
