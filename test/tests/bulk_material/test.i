[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 100
    ny = 100
  []
[]

[Variables]
  [v]
  []
[]

[Kernels]
  [dt]
    type = TimeDerivative
    variable = v
  []
  [diff]
    type = Diffusion
    variable = v
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = v
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = v
    boundary = right
    value = 1
  []
[]

[UserObjects]
  [bulk]
    type = BulkMaterialTest
    var1 = v
    prop1 = tensor
    prop2 = number
    execute_on = 'LINEAR'
  []
[]

[Materials]
  [prop1]
    type = GenericConstantRankTwoTensor
    tensor_name = tensor
    tensor_values = '1 2 3 4 5 6 7 8 9'
  []
  [prop2]
    type = GenericFunctionMaterial
    prop_names = number
    prop_values = 'x^2+sin(y*3)+cos(t*10)'
  []

  [test]
    type = BulkTestMaterial
    var1 = v
    prop1 = tensor
    prop2 = number
    bulk_uo = bulk
  []
[]

[Postprocessors]
  [average]
    type = ElementAverageMaterialProperty
    mat_prop = bulk_out
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  dt = 0.1
  num_steps = 3
[]
