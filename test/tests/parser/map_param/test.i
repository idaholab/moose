[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[Variables]
  [u]
  []
  [v]
  []
  [w]
  []
[]

[Kernels]
  inactive = 'odd_entries bad_value'
  [diff]
    type = ADDiffusion
    variable = u
  []
  [diff_v]
    type = ADDiffusion
    variable = v
  []
  [diff_w]
    type = ADDiffusion
    variable = w
  []
  [map]
    type = MapMultiplyCoupledVars
    variable = u
    v = v
    w = w
    coupled_var_multipliers = 'v 2 w 3'
    dummy_string_to_string_map = 'a 1 b c'
    dummy_ullong_to_uint_map = '5000000000 1 2 0'
    dummy_uint_to_uint_map = '50 1 2 0'
    dummy_ulong_to_uint_map = '50 1 2 0'
  []
  [odd_entries]
    type = MapMultiplyCoupledVars
    variable = u
    v = v
    w = w
    coupled_var_multipliers = 'v 2 w'
    dummy_string_to_string_map = 'a 1 b c'
    dummy_ullong_to_uint_map = '5000000000 1 2 0'
    dummy_uint_to_uint_map = '50 1 2 0'
    dummy_ulong_to_uint_map = '50 1 2 0'
  []
  [bad_value]
    type = MapMultiplyCoupledVars
    variable = u
    v = v
    w = w
    coupled_var_multipliers = 'v 2 w a'
    dummy_string_to_string_map = 'a 1 b c'
    dummy_ullong_to_uint_map = '5000000000 1 2 0'
    dummy_uint_to_uint_map = '50 1 2 0'
    dummy_ulong_to_uint_map = '50 1 2 0'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
  [left_v]
    type = DirichletBC
    variable = v
    boundary = left
    value = 0
  []
  [right_v]
    type = DirichletBC
    variable = v
    boundary = right
    value = 1
  []
  [left_w]
    type = DirichletBC
    variable = w
    boundary = left
    value = 0
  []
  [right_w]
    type = DirichletBC
    variable = w
    boundary = right
    value = 1
  []
[]



[Executioner]
  type = Steady
  solve_type = 'Newton'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
