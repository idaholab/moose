[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = '${fparse 2*pi}'
    ymin = 0
    ymax = '${fparse 2*pi}'
    nx = 10
    ny = 10
  []
[]

[Problem]
  kernel_coverage_check = false
[]

[Variables]
  [dummy]
    type = MooseVariableFVReal
    initial_condition = 1
  []
[]

[Postprocessors]
  [h]
    type = AverageElementSize
  []
  [L2u]
    type = TestFaceCenteredMapFunctor
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
