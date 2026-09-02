[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 3
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diffusion]
    type = Diffusion
    variable = u
  []
  [reaction]
    type = Reaction
    variable = u
    rate = -1
    extra_vector_tags = eigen
  []
[]

[BCs]
  [homogeneous]
    type = DirichletBC
    variable = u
    boundary = 'left right top bottom'
    value = 0
  []
  [eigen]
    type = EigenDirichletBC
    variable = u
    boundary = 'left right top bottom'
  []
[]

[Executioner]
  type = Eigenvalue
  solve_type = PJFNK
  nl_abs_tol = 1e-12
[]

[MultiApps]
  inactive = 'simple pimple'
  [simple]
    type = FullSolveMultiApp
    input_files = simple.i
    execute_on = timestep_end
  []
  [pimple]
    type = FullSolveMultiApp
    input_files = pimple.i
    execute_on = timestep_end
  []
[]
