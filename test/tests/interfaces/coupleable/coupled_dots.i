[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 2
  []
[]

[Functions]
  [linear]
    type = ParsedFunction
    expression = 'x + 10*t + 2*t*t'
  []
[]

[AuxVariables]
  [base]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [base_aux]
    type = FunctionAux
    function = 'linear'
    variable = 'base'
  []
[]

[Executioner]
  type = Transient
  num_steps = 3

  [TimeIntegrator]
    type = CentralDifference
  []
[]

[Problem]
  solve = false

[]

[Materials]
  [coupled]
    type = CoupledValuesMaterial
    variable = 'base'
  []
  [ad_coupled]
    type = ADCoupledValuesMaterial
    variable = 'base'
    declare_suffix = 'ad'
  []
[]

[Postprocessors]
  [dot]
    type = ElementAverageMaterialProperty
    mat_prop = 'base_dot'
  []
  [dot_dot]
    type = ElementAverageMaterialProperty
    mat_prop = 'base_dot_dot'
  []
  [ad_dot]
    type = ADElementAverageMaterialProperty
    mat_prop = 'base_dot_ad'
  []
  [ad_dot_dot]
    type = ADElementAverageMaterialProperty
    mat_prop = 'base_dot_dot_ad'
  []
[]

[Outputs]
  csv = true
[]
