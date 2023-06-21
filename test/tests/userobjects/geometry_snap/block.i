[Mesh]
  [gen]
    type = PolyLineMeshGenerator
    points = "0 0 0
              0 1 0
              1 1 0
              1 0 0"
    loop = true
  []
[]

[Variables]
  [u]
    initial_condition = 1
  []
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Executioner]
  type = Steady
[]

[UserObjects]
  [sphere]
    type = GeometrySphere
    center = '0.5 0.5 0'
    radius = 0.7071
    block = 0
  []
[]

[Adaptivity]
  [Markers]
    [const]
      type = UniformMarker
      mark = REFINE
    []
  []
  marker = const
  steps = 3
[]

[Outputs]
  [out]
    type = Exodus
  []
[]
