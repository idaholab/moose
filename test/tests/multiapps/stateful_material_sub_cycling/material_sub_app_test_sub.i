[GlobalParams]
  dim = 2
[]

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
  [./prop_to_time]
    type = ProportionalToTime
  [../]
[]

[Executioner]
  type = Transient
  start_time = 0.0
[]

[Postprocessors]
  [./matl_integral]
    type = ElementIntegralMaterialProperty
    mat_prop = prop_to_time
    execute_on = timestep_end
    outputs = 'console csv'
  [../]
[]

[Outputs]
  csv = true
  color = false
  [./exodus]
    type = Exodus
    execute_on = 'timestep_end'
  [../]
  [./console]
    type = Console
    perf_log = true
    max_rows = 10
  [../]
[]

