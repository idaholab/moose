[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Adaptivity]
  [./Indicators]
    [./indicator]
      type = MaterialTestIndicator
      property = 'prop'
    [../]
  [../]
[]

[Materials]
  [./mat]
    type = GenericFunctionMaterial
    prop_names = 'prop'
    prop_values = 'func'
  [../]
[]

[Functions]
  [./func]
    type = ParsedFunction
    value = 'if(y<0.5,1980,1949)'
  [../]
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
