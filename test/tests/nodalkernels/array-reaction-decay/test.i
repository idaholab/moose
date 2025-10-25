[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 2
  []
[]

[Variables]
  [u]
    components = 2
  []
[]

[ICs]
  [u]
    type = ArrayConstantIC
    variable = u
    value = '2 2'
  []
[]

[NodalKernels]
  [time]
    type = ArrayTimeDerivativeNodalKernel
    variable = u
  []
  [reaction]
    type = ArrayReactionNodalKernel
    variable = u
    coeff = '4 2'
  []
[]

[AuxVariables]
  [u0]
  []
  [u1]
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
[]

[Outputs]
  csv = true
[]

[Executioner]
  type = Transient
  num_steps = 2
[]
