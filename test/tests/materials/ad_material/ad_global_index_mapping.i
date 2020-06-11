[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  second_order = true
[]

[Variables]
  [u]
    initial_condition = 1
  []
  [v]
    initial_condition = 1
    order = SECOND
  []
[]

[Kernels]
  [u_diff]
    type = ADMatDiffusion
    variable = u
    diffusivity = diffusivity
  []
  [v_diff]
    type = ADMatDiffusion
    variable = v
    diffusivity = diffusivity
  []
[]

[BCs]
  [left_u]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right_u]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
  [left_v]
    type = DirichletBC
    variable = v
    boundary = left
    value = 1
  []
  [right_v]
    type = DirichletBC
    variable = v
    boundary = right
    value = 0
  []
[]

[Materials]
  [ad_coupled_mat]
    type = ADCheckGlobalToDerivativeMap
    u = u
    v = v
    mat_prop = diffusivity
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
