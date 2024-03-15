[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 10
  ymin = 0
  ymax = 10
  elem_type = QUAD4
  nx = 2
  ny = 2
[]

[Variables]
  [u]
  []
[]

[Problem]
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []

  [rhs]
    type = CoefReaction
    variable = u
    coefficient = -1.0
    extra_vector_tags = 'eigen'
  []
[]

[BCs]
  [homogeneous]
    type = DirichletBC
    variable = u
    boundary = '0 1 2 3'
    value = 0
  []
  [eigen]
    type = EigenDirichletBC
    variable = u
    boundary = '0 1 2 3'
  []
[]

[Executioner]
  type = Eigenvalue
  solve_type = PJFNK
  nl_max_its = 1
  nl_rel_tol = 1e-50
  nl_abs_tol = 1e-50
  free_power_iterations = 0
[]
