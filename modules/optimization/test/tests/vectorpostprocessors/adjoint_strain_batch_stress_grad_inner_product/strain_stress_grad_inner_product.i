[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
  []
[]

[Variables]
  [v]
    initial_condition = 1
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
  # assign value using the following object
  [batch]
    type = OptimizationBatchPropertyDerivativeTest
    batch_deriv_uo = batch_deriv
    prop = tensor
    execute_on = 'LINEAR'
  []
[]

[Materials]
  [prop1]
    type = GenericConstantRankTwoTensor
    tensor_name = tensor
    tensor_values = '1 6 5 6 2 4 5 4 3'
  []
  [prop2]
    type = GenericFunctionMaterial
    prop_names = number
    prop_values = '1.0'
  []
  [prop3]
    type = GenericConstantRankTwoTensor
    tensor_name = dummy_strain
    tensor_values = '10 11 12 13 14 15 16 17 18'
  []
[]

[Functions]
  [fcn]
    type = NearestReporterCoordinatesFunction
    x_coord_name = param/coordx
    y_coord_name = param/coordy
    value_name = param/value
  []
[]

[Reporters]
  [param]
    type = ConstantReporter
    real_vector_names = 'coordx coordy value'
    real_vector_values = '0 0.5 1; 0 0.5 1; 1.0 2.0 3.0'
    outputs = none
  []
[]

[VectorPostprocessors]
  [inner_product]
    type = AdjointStrainBatchStressGradInnerProduct
    adjoint_strain_name = dummy_strain
    stress_derivative = batch_deriv
    function = fcn
    variable = v
  []
[]

[Executioner]
  type = Transient
  dt = 0.1
  end_time = 0.1
[]

[Outputs]
  csv = true
  execute_on = final
[]
