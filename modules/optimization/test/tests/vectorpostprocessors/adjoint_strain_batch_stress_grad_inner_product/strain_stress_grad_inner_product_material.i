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

[Materials]
  [prop1] # dummy stress deriv
    type = GenericConstantSymmetricRankTwoTensor
    tensor_name = tensor
    tensor_values = '1 2 3 4 5 6'
  []
  [prop2]
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
    type = AdjointStrainSymmetricStressGradInnerProduct
    adjoint_strain_name = dummy_strain
    stress_derivative_name = tensor
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
