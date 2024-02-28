[GlobalParams]
  displacements = 'dx dy'
[]

[Problem]
  solve = false
[]

[Mesh]
  [generated_mesh]
    type = FileMeshGenerator
    file = inclined_geom.e
  []
[]

[AuxVariables]
  [dx]
  []

  [dy]
  []
[]

[ICs]
  [dx_ic]
    type = FunctionIC
    variable = dx
    function = 'r := sqrt(x*x+y*y); phi := 2 * pi * r; 0.8660254037844408 * cos(phi) - 0.5 * sin(phi)'
  []

  [dy_ic]
    type = FunctionIC
    variable = dy
    function = 'r := sqrt(x*x+y*y); phi := 2 * pi * r; 0.8660254037844408 * sin(phi) + 0.5 * cos(phi)'
  []
[]

[Postprocessors]
  [top_area]
    type = AreaPostprocessor
    boundary = top
  []

  [top_0]
    type = NormalBoundaryDisplacement
    value_type = average
    boundary = top
  []

  [top_1]
    type = NormalBoundaryDisplacement
    value_type = absolute_average
    boundary = top
  []

  [top_2]
    type = NormalBoundaryDisplacement
    value_type = max
    boundary = top
  []

  [top_3]
    type = NormalBoundaryDisplacement
    value_type = absolute_max
    boundary = top
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
