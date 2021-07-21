# The PorousFlowElementLength is used to compute element lengths according to different directions, in 3D
[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    xmin = -1
    xmax = 1
    ny = 1
    ymin = 0
    ymax = 4
    nz = 1
    zmin = -2
    zmax = 4
  []
[]

[Variables]
  [dummy]
  []
[]

[Kernels]
  [dummy]
    type = Diffusion
    variable = dummy
  []
[]

[AuxVariables]
  [d100]
    family = MONOMIAL
    order = CONSTANT
  []
  [d010]
    family = MONOMIAL
    order = CONSTANT
  []
  [d001]
    family = MONOMIAL
    order = CONSTANT
  []
  [d110]
    family = MONOMIAL
    order = CONSTANT
  []
  [d1n10]
    family = MONOMIAL
    order = CONSTANT
  []
  [dn1n10]
    family = MONOMIAL
    order = CONSTANT
  []
  [d111]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [d100]
    type = PorousFlowElementLength
    direction = '1 0 0'
    variable = d100
  []
  [d010]
    type = PorousFlowElementLength
    direction = '0 10 0'
    variable = d010
  []
  [d001]
    type = PorousFlowElementLength
    direction = '0 0 2'
    variable = d001
  []
  [d110]
    type = PorousFlowElementLength
    direction = '1 1 0'
    variable = d110
  []
  [d1n10]
    type = PorousFlowElementLength
    direction = '1 -1 0'
    variable = d1n10
  []
  [dn1n10]
    type = PorousFlowElementLength
    direction = '-1 -1 0'
    variable = dn1n10
  []
  [d111]
    type = PorousFlowElementLength
    direction = '4 4 4'
    variable = d111
  []
[]

[Postprocessors]
  [d100]
    type = PointValue
    point = '0 0 0'
    variable = d100
  []
  [d010]
    type = PointValue
    point = '0 0 0'
    variable = d010
  []
  [d001]
    type = PointValue
    point = '0 0 0'
    variable = d001
  []
  [d110]
    type = PointValue
    point = '0 0 0'
    variable = d110
  []
  [d1n10]
    type = PointValue
    point = '0 0 0'
    variable = d1n10
  []
  [dn1n10]
    type = PointValue
    point = '0 0 0'
    variable = dn1n10
  []
  [d111]
    type = PointValue
    point = '0 0 0'
    variable = d111
  []
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 1
[]

[Outputs]
  csv = true
[]

