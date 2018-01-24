[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmin = 0.0
  xmax = 1.0
  ymin = 0.0
  ymax = 1.0
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./b]
    family = SCALAR
    order = SIXTH
  [../]
[]

[Kernels]
  [./diffusion]
    type = Diffusion
    variable = u
  [../]
  [./time]
    type = TimeDerivative
    variable = u
  [../]
[]

[Executioner]
  type = Transient
[]

[Outputs]
  hide = 'u'
  exodus = true
[]
