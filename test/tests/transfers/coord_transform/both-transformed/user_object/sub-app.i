[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 0
  ymin = 0
  ymax = 1
  nx = 10
  ny = 10
  alpha_rotation = 90
[]

[Variables]
  [v][]
[]

[AuxVariables]
  [sub_app_var][]
  [sub_app_var_elem]
    order = CONSTANT
    family = MONOMIAL
  []
  [check][]
[]

[AuxKernels]
  [uo]
    type = SpatialUserObjectAux
    variable = check
    user_object = 'sub_app_uo'
  [][]

[UserObjects]
  [sub_app_uo]
    type = LayeredAverage
    direction = y
    variable = v
    num_layers = 5
    execute_on = TIMESTEP_END
    use_displaced_mesh = true
  []
[]

[Kernels]
  [diff_v]
    type = Diffusion
    variable = v
  []
[]

[BCs]
  [left_v]
    type = DirichletBC
    variable = v
    boundary = bottom
    value = 0
  []
  [right_v]
    type = DirichletBC
    variable = v
    boundary = top
    value = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
