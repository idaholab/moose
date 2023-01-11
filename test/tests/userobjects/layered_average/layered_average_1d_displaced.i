# This tests that Layered user objects work with displaced meshes.  Originally,
# the mesh is aligned with x-axis.  Then we displace the mesh to be aligned with
# z-axis and sample along the z-direction.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 5
  elem_type = EDGE2
  displacements = 'disp_x disp_y disp_z'
[]

[Functions]
  [./left_fn]
    type = ParsedFunction
    expression = 't + 1'
  [../]

  [./disp_x_fn]
    type = ParsedFunction
    expression = '-x'
  [../]
  [./disp_z_fn]
    type = ParsedFunction
    expression = 'x'
  [../]
[]

[AuxVariables]
  [./la]
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
  [./la_ak]
    type = SpatialUserObjectAux
    variable = la
    user_object = la_uo
    execute_on = TIMESTEP_END
    use_displaced_mesh = true
  [../]

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
  [./la_uo]
    type = LayeredAverage
    direction = z
    variable = u
    num_layers = 5
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
    type = FunctionDirichletBC
    variable = u
    boundary = left
    function = left_fn
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 0
  [../]
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 2
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
