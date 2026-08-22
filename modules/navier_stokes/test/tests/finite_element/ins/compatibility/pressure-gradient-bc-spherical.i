[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 1
  xmax = 2
  nx = 2
  coord_type = RSPHERICAL
[]

[Variables]
  [velocity]
    family = MONOMIAL
    order = FIRST
  []
  [pressure]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Kernels]
  [velocity]
    type = Reaction
    variable = velocity
  []
  [pressure]
    type = Reaction
    variable = pressure
  []
[]

[BCs]
  [pressure]
    type = INSPressureGradientBC
    boundary = left
    variable = velocity
    pressure = pressure
    component = 0
  []
[]

[Executioner]
  type = Steady
[]
