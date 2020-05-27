[Functions]
  [force]
    type = ParsedFunction
    value = '0.0333333333333333*exp(9*sin(2*x*pi)*sin(2*y*pi)) + 8*pi^2*sin(2*x*pi)*sin(2*y*pi) - 0.0333333333333333'
  []
  [exact]
    type = ParsedFunction
    value = 'sin(2*x*pi)*sin(2*y*pi)'
  []
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 25
    xmin = 0
    xmax = 1
    ny = 25
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
    function = force
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
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Postprocessors]
  [error]
    type = ElementL2Error
    function = exact
    variable = U
  []
  [h]
    type = AverageElementSize
  []
[]

[Outputs]
[]
