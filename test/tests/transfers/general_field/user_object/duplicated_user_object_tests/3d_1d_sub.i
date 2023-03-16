[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  elem_type = EDGE2
  displacements = 'disp_x disp_y disp_z'
[]

[Functions]
  [./disp_x_fn]
    type = ParsedFunction
    value = '-x'
  [../]
  [./disp_z_fn]
    type = ParsedFunction
    value = 'x'
  [../]
[]

[AuxVariables]
  [./sub_app_var]
    family = MONOMIAL
    order = CONSTANT
  [../]

  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]

[AuxKernels]
  [./disp_x_ak]
    type = FunctionAux
    variable = disp_x
    function = 'disp_x_fn'
  [../]
  [./disp_y_ak]
    type = ConstantAux
    variable = disp_y
    value = 0
  [../]
  [./disp_z_ak]
    type = FunctionAux
    variable = disp_z
    function = 'disp_z_fn'
  [../]
[]

[UserObjects]
  [./sub_app_uo]
    type = LayeredAverage
    direction = z
    variable = u
    num_layers = 10
    execute_on = TIMESTEP_END
    use_displaced_mesh = true
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./td]
    type = TimeDerivative
    variable = u
  [../]

  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 1
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 2
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
