[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 0
  ymin = 0
  ymax = 1
  nx = 10
  ny = 10
  alpha_rotation = -90
[]

[Variables]
  [v][]
[]

[AuxVariables]
  [v_elem]
    order = CONSTANT
    family = MONOMIAL
  []
  [w][]
  [w_elem]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [v_elem]
    type = SelfAux
    v = v
    variable = v_elem
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
