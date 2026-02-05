[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 2
  []
[]

[Variables]
  [u]
    components = 10
  []
[]

[ICs]
  [u]
    type = ArrayConstantIC
    variable = u
    value = '1 0 0 0 0 0 0 0 0 0'
  []
[]

[NodalKernels]
  [time]
    type = ArrayTimeDerivativeNodalKernel
    variable = u
  []
  [reaction]
    type = ArrayCoupledReactionNodalKernel
    variable = u
    coeff = '1 1 1 0.1 1 1 1 1 1'
  []
[]

[AuxVariables]
  [u0]
  []
  [u1]
  []
  [u2]
  []
  [u3]
  []
  [u4]
  []
  [u5]
  []
  [u6]
  []
  [u7]
  []
  [u8]
  []
  [u9]
  []
[]

[AuxKernels]
  [u0]
    type = ArrayVariableComponent
    array_variable = u
    variable = u0
    component = 0
  []
  [u1]
    type = ArrayVariableComponent
    array_variable = u
    variable = u1
    component = 1
  []
  [u2]
    type = ArrayVariableComponent
    array_variable = u
    variable = u2
    component = 2
  []
  [u3]
    type = ArrayVariableComponent
    array_variable = u
    variable = u3
    component = 3
  []
  [u4]
    type = ArrayVariableComponent
    array_variable = u
    variable = u4
    component = 4
  []
  [u5]
    type = ArrayVariableComponent
    array_variable = u
    variable = u5
    component = 5
  []
  [u6]
    type = ArrayVariableComponent
    array_variable = u
    variable = u6
    component = 6
  []
  [u7]
    type = ArrayVariableComponent
    array_variable = u
    variable = u7
    component = 7
  []
  [u8]
    type = ArrayVariableComponent
    array_variable = u
    variable = u8
    component = 8
  []
  [u9]
    type = ArrayVariableComponent
    array_variable = u
    variable = u9
    component = 9
  []
[]

[Postprocessors]
  [u0]
    type = ElementAverageValue
    variable = u0
  []
  [u1]
    type = ElementAverageValue
    variable = u1
  []
  [u2]
    type = ElementAverageValue
    variable = u2
  []
  [u3]
    type = ElementAverageValue
    variable = u3
  []
  [u4]
    type = ElementAverageValue
    variable = u4
  []
  [u5]
    type = ElementAverageValue
    variable = u5
  []
  [u6]
    type = ElementAverageValue
    variable = u6
  []
  [u7]
    type = ElementAverageValue
    variable = u7
  []
  [u8]
    type = ElementAverageValue
    variable = u8
  []
  [u9]
    type = ElementAverageValue
    variable = u9
  []
[]

[Outputs]
  csv = true
[]

[Executioner]
  type = Transient
  dt = 0.01
  num_steps = 100
[]
