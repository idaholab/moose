[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    elem_type = C0POLYGON
    nx = 2
    ny = 3
  []
  [feg]
    type = AdvancedExtruderGenerator
    input = gmg
    direction = '0 0 1'
    num_layers = 1
    heights = 1
    top_boundary = 100
    bottom_boundary = 200
  []
  [output_tetrahedralization]
    type = ElementsToTetrahedronsConverter
    input = 'feg'
  []
[]

[AuxVariables]
  [xyz]
  order = CONSTANT
  family = MONOMIAL
    [AuxKernel]
      type = ParsedAux
      expression = 'x + 2*y + 3*z'
      use_xyzt = true
      execute_on = TIMESTEP_END
    []
  []
[]

[GlobalParams]
  min_x = -0.5
  max_x = 0.5
  min_y = -0.5
  max_y = 0.5

  resolution = 50
  frame_center = '0.5 0.5 0.5'

  min = -10
  max = 12
[]

[Outputs]
  [xy_base]
    type = PNGOutput
    variable = xyz
    color = 'BWR'
    execute_on = FINAL
    frame_center = '0.5 0.5 0.2'
  []
  [xy]
    type = PNGOutput
    variable = xyz
    color = 'BWR'
    execute_on = FINAL
    frame_center = '0.5 0.5 0.8'
  []
  [xz]
    type = PNGOutput
    variable = xyz
    color = 'BWR'
    execute_on = FINAL
    first_axis = '1 0 0'
    second_axis = '0 0 1'
  []
  [yz]
    type = PNGOutput
    variable = xyz
    color = 'BWR'
    execute_on = TIMESTEP_END
    first_axis = '0 1 0'
    second_axis = '0 0 1'
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Postprocessors]
  [n_elems]
    type = NumElements
  []
  [min]
    type = ElementExtremeValue
    variable = 'xyz'
    value_type = 'min'
  []
  [max]
    type = ElementExtremeValue
    variable = 'xyz'
  []
[]

[VectorPostprocessors]
  [div_out]
    type = ElementValueSampler
    variable = 'xyz'
    sort_by = 'id'
  []
[]
