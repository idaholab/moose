[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 100
  xmin = -2
  xmax = 2
[]

[Variables]
  [u]
  []
[]

[ICs]
  [u_ic]
    type = FunctionIC
    variable = u
    function = u_ic_fn
  []
[]

[Functions]
  [u_ic_fn]
    type = ParsedFunction
    expression = 'x'
  []
[]

[Materials]
  [test_mat]
    type = SmoothTransitionTestMaterial
    transition_type = cubic
    var = u
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[VectorPostprocessors]
  [test_vpp]
    type = Sampler1DReal
    block = 0
    property = mymatprop
    sort_by = x
    execute_on = 'INITIAL'
  []
[]

[Outputs]
  csv = true
  file_base = 'cubic_nonad'
  execute_on = 'INITIAL'
[]
