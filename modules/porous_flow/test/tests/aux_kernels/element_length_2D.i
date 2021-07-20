# The PorousFlowElementLength is used to compute element lengths according to different directions, in 2D
[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    ny = 1
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
  [ten]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 10
  []
  [zero]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 0
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
    direction = 'zero ten zero'
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
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 1
[]

[Outputs]
  csv = true
[]

