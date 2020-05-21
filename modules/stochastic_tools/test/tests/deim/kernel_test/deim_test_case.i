[Functions]
  [source]
    type = ParsedFunction
    value = "100 * sin(2 * pi * x) * sin(2 * pi * y)"
  []
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 100
    xmin = 0
    xmax = 1
    ny = 100
    ymin = 0
    ymax = 1
  []
[]

[Variables]
  [U]
    family = lagrange
    order = first
  []
[]

[Kernels]
  [diffusion]
    type = Diffusion
    variable = U
  []
  [nonlin_function]
    type = ExponentialReaction
    variable = U
    mu1 = 0.3
    mu2 = 9
  []
  [source]
    type = BodyForce
    variable = U
    function = source
  []
[]

[BCs]
  [dirichlet_all]
    type = DirichletBC
    variable = U
    boundary = 'left right top bottom'
    value = 0
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 101'
[]

[Outputs]
  exodus = true
[]
