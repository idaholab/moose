[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 2
  ny = 2
  nz = 2
  xmin = 0.0
  xmax = 4.0
  ymin = 0.0
  ymax = 6.0
  zmin = 0.0
  zmax = 10.0
[]

[FunctorMaterials]
  [fmat]
    type = GenericFunctorMaterial
    prop_values = '1 2'
    prop_names = 'prop1 prop2'
  []
[]

[Postprocessors]
  [get_prop1]
    type = ElementExtremeFunctorValue
    functor = prop1
    value_type = max
    execute_on = 'INITIAL'
  []
  [get_prop2]
    type = ElementExtremeFunctorValue
    functor = prop2
    value_type = max
    execute_on = 'INITIAL'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Debug]
  show_functors = true
[]

[Outputs]
  csv = true
  execute_on = 'INITIAL'
[]
