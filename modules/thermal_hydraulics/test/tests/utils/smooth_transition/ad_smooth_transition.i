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
    type = ADSmoothTransitionTestMaterial
    transition_type = weighted
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
    type = ADSampler1DReal
    block = 0
    property = myadmatprop
    sort_by = x
    execute_on = 'INITIAL'
  []
[]

[Outputs]
  csv = true
  file_base = 'ad_weighted'
  execute_on = 'INITIAL'
[]
