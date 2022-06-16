[Problem]
  solve = false
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[AuxVariables]
  [./x]
    family = SCALAR
    order = FIRST
  [../]
[]

[AuxScalarKernels]
  [./const_x]
    type = ConstantScalarAux
    variable = x
    value = 0
  [../]
[]

[Materials]
  [./stateful]
    type = StatefulMaterial
  [../]
[]

[Executioner]
  type = Transient
[]

[Postprocessors]
  [./matl_integral]
    type = ElementIntegralMaterialProperty
    mat_prop = diffusivity
    execute_on = timestep_end
    outputs = 'console csv'
  [../]
[]

[Outputs]
  csv = true
[]
