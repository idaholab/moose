#
# mechanics part of the tightly coupled problem
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
  [./disp_x_exact]
    type = ParsedFunction
    value = (t*x)/5
  [../]
  [./disp_x_ffn]
    type = ParsedFunction
    value = x/5
  [../]

  [./disp_y_exact]
    type = ParsedFunction
    value = t*x*(y-0.5)/5
  [../]
  [./disp_y_ffn]
    type = ParsedFunction
    value = x*(y-0.5)/5
  [../]
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[Kernels]
  # disp_x
  [./disp_x_td]
    type = TimeDerivative
    variable = disp_x
  [../]
  [./disp_x_diff]
    type = Diffusion
    variable = disp_x
  [../]
  [./disp_x_ffn]
    type = UserForcingFunction
    variable = disp_x
    function = disp_x_ffn
  [../]

  # disp_y
  [./disp_y_td]
    type = TimeDerivative
    variable = disp_y
  [../]
  [./disp_y_diff]
    type = Diffusion
    variable = disp_y
  [../]
  [./disp_y_ffn]
    type = UserForcingFunction
    variable = disp_y
    function = disp_y_ffn
  [../]
[]

[BCs]
  [./disp_x_all]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = '0 1 2 3'
    function = disp_x_exact
  [../]

  [./disp_y_all]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = '0 1 2 3'
    function = disp_y_exact
  [../]
[]

[Executioner]
  type = Transient
  start_time = 0
  dt = 0.1
  num_steps = 5
[]

[Output]
  exodus = true
  output_initial = true
[]
