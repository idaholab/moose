[Mesh]
  [cc]
    type = ConcentricCircleMeshGenerator
    num_sectors = 6
    radii = '1'
    rings = '2'
    has_outer_square = false
    portion = left_half
    preserve_volumes = true
  []
  [coating]
    type = Conformal2DCoatingGenerator
    input = 'cc'
    thickness = 0.1
    num_layers = 3
    keep_input = true
  []
[]

[Problem]
  solve = false
[]

[AuxVariables]
  [unity]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 1.0
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Postprocessors]
  [int_unity_substrate]
    type = ElementIntegralVariablePostprocessor
    variable = unity
    block = 1
  []
  [int_unity_coating]
    type = ElementIntegralVariablePostprocessor
    variable = unity
    block = 0
  []
[]

[Outputs]
  file_base = 'coating_stitch_outward'
  csv = true
  execute_on = 'FINAL'
[]
