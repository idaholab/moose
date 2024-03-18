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
  [null]
    type = NullKernel
    variable = v
  []
[]


[UserObjects]
  [batch_deriv]
    type = BatchPropertyDerivativeRankTwoTensorReal
    material_property = number
  []
  [batch]
    type = BatchPropertyDerivativeTest
    batch_deriv_uo = batch_deriv
    prop = tensor
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
    type = BatchTestPropertyDerivative
    prop = tensor
    batch_uo = batch
  []
[]

[Postprocessors]
  [average]
    type = ElementAverageMaterialProperty
    mat_prop = batch_out
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]
