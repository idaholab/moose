[Mesh]
  [ccmg]
    type = ConcentricCircleMeshGenerator
    num_sectors = 6
    radii = '0.2546 0.3368 0.3600 0.3818 0.3923 0.4025 0.4110 0.4750'
    rings = '5 3 2 1 1 1 1 3 5'
    has_outer_square = on
    pitch = 1.42063
    #portion = left_half
    preserve_volumes = off
    smoothing_max_it = 3
  []
[]

[AuxVariables]
  [winding_order]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [winding_order]
    type = WindingOrder
    variable = winding_order
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Postprocessors]
  [min_winding_order]
    type = ElementExtremeValue
    value_type = min
    variable = winding_order
  []
  [max_winding_order]
    type = ElementExtremeValue
    value_type = max
    variable = winding_order
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
  csv = true
  hide = winding_order
[]
