[Mesh]
  [outer_bdy]
    type = PolyLineMeshGenerator
    points = '0.0 0.0 0.0
              2.0 0.0 0.0
              2.0 1.0 0.0
              1.0 1.0 0.0
              1.0 2.0 0.0
              0.0 2.0 0.0'
    loop = true
  []
  [triang]
    type = XYFrontalDelaunayGenerator
    boundary = 'outer_bdy'
    refine_boundary = true
    desired_area = 0.02
    output_subdomain_name = 'triangles'
  []
[]

[Problem]
  solve = false
[]

[AuxVariables]
  [quality]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [qa]
    type = ElementQualityAux
    variable = quality
    metric = SHAPE
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Postprocessors]
  [area]
    type = VolumePostprocessor
    outputs = csv
  []
  [avg_quality]
    type = ElementAverageValue
    variable = quality
    outputs = csv
  []
  [elem_size]
    type = AverageElementSize
    outputs = csv
  []
[]

[Outputs]
  csv = true
  execute_on = 'FINAL'
[]
