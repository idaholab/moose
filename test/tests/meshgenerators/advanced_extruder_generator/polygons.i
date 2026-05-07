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
  allow_renumbering = false
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

[Outputs]
  csv = true
[]
