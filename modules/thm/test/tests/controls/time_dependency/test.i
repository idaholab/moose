[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Components]
[]

[ControlLogic]
  [ctrl]
    type = ScalingControl
    scale = 2
    initial = 1
  []
[]

[Postprocessors]
  [control_value]
    type = RealControlDataValuePostprocessor
    control_data_name = ctrl:value
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 4
[]

[Outputs]
  csv = true
[]
