[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 1
[]

[Functions]
  [func]
    type = ConstantFunction
    value = 5
  []
[]

[VectorPostprocessors]
  [vpp]
    type = LineFunctionSampler
    functions = func
    start_point = '0 0 0'
    end_point = '1 0 0'
    num_points = 4
    sort_by = x
    execute_on = final
  []
[]

[Outputs]
  [csv]
    type = CSV
    show = 'vpp'
    execute_on = final
  []
[]
