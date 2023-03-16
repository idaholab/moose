[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 5
  []
[]

[AuxVariables]
  [fl]
    type = MooseVariableFVReal
  []
  [T]
    type = MooseVariableFVReal
  []
[]

[ICs]
  [FunctionIC]
    type = FunctionIC
    variable = T
    function = '10 + x'
  []
[]

[AuxKernels]
  [liquid_fraction]
    type = NSLiquidFractionAux
    variable = fl
    temperature = T
    T_liquidus = 20
    T_solidus = 10
  []
[]

[VectorPostprocessors]
  [T]
    type = LineValueSampler
    start_point = '0.1 0 0'
    end_point = '0.9 0 0'
    num_points = 5
    variable = T
    sort_by = x
  []
  [fl]
    type = LineValueSampler
    start_point = '0.1 0 0'
    end_point = '0.9 0 0'
    num_points = 5
    variable = fl
    sort_by = x
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
