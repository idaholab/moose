#
# thermal part of the tightly coupled problem
#
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 10
  ny = 10
  elem_type = QUAD4
  displacements = 'disp_x disp_y'
[]

[Functions]
  [./temp_exact]
    type = ParsedFunction
    value = t*x
  [../]
  [./temp_ffn]
    type = ParsedFunction
    value = x
  [../]
  []

[Variables]
  [./temp]
  [../]
[]

[Kernels]
  # temp
  [./temp_td]
    type = TimeDerivative
    variable = temp
  [../]
  [./temp_diff]
    type = Diffusion
    variable = temp
  [../]
  [./temp_ffn]
    type = UserForcingFunction
    variable = temp
    function = temp_ffn
  [../]
[]

[BCs]
  [./temp_all]
    type = FunctionDirichletBC
    variable = temp
    boundary = '0 1 2 3'
    function = temp_exact
  [../]
[]

[Executioner]
  type = Transient
  start_time = 0
  dt = 0.1
  num_steps = 5
[]

[Outputs]
  exodus = true
  output_initial = true
  [./console]
    type = Console
    perf_log = true
  [../]
[]