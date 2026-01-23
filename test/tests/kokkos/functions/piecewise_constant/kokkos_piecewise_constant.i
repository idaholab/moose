[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [coef]
  []
[]

[UserObjects]
  [json]
    type = JSONFileReader
    filename = 'xy.json'
  []
[]

[Functions]
  [func_x_y]
    type = KokkosPiecewiseConstant
    x = '0.0 0.5'
    y = '2.0 3.0'
  []
  [func_xy_data]
    type = KokkosPiecewiseConstant
    xy_data = '0.0 2.0
               0.5 3.0'
  []
  [func_csv]
    type = KokkosPiecewiseConstant
    data_file = xy.csv
  []
  [func_json]
    type = KokkosPiecewiseConstant
    json_uo = json
    x_keys = 'data x value'
    y_keys = 'data y value'
  []
[]

[Kernels]
  [diff]
    type = KokkosFuncCoefDiffusion
    variable = u
    coef = func_x_y
  []
  [time]
    type = KokkosTimeDerivative
    variable = u
  []
[]

[AuxKernels]
  [coef_aux]
    type = KokkosFunctionAux
    variable = coef
    function = func_x_y
  []
[]

[BCs]
  [left]
    type = KokkosDirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = KokkosNeumannBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Postprocessors]
  [coef]
    type = KokkosElementIntegralVariablePostprocessor
    variable = coef
  []
[]

[Executioner]
  type = Transient
  num_steps = 10
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
