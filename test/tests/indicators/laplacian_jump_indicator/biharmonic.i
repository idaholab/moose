[GlobalParams]
  # Parameters used by Functions.
  vars   = 'c'
  vals   = '50'
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -.5
  xmax = .5
  ymin = -.5
  ymax = .5
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
    order = THIRD
    family = HERMITE
  [../]
[]

[Kernels]
  [./biharmonic]
    type = Biharmonic
    variable = u
  [../]
  [./body_force]
    type = BodyForce
    variable = u
    function = forcing_func
  [../]
[]

[BCs]
  active = 'all_value all_flux'
  [./all_value]
    type = FunctionPenaltyDirichletBC
    variable = u
    boundary = 'left right top bottom'
    function = u_func
    penalty = 1e10
  [../]
  [./all_flux]
    type = FunctionPenaltyFluxBC
    variable = u
    boundary = 'left right top bottom'
    function = u_func
    penalty = 1e10
  [../]
  [./all_laplacian]
    type = BiharmonicLapBC
    variable = u
    boundary = 'left right top bottom'
    laplacian_function = lapu_func
  [../]
[]

[Adaptivity]
  [Indicators]
    [error]
      type = LaplacianJumpIndicator
      variable = u
      scale_by_flux_faces = true
    []
  []
[]

[Executioner]
  type = Steady

  # Note: the unusually tight tolerances here are due to the penalty
  # BCs (currently the only way of accurately Dirichlet boundary
  # conditions on Hermite elements in MOOSE).
  nl_rel_tol = 1.e-15
  l_tol = 1.e-15

  # We have exact Jacobians
  solve_type = 'NEWTON'

  # Use 6x6 quadrature to ensure the forcing function is integrated
  # accurately.
  [./Quadrature]
    type = GAUSS
    order = ELEVENTH
  [../]
[]

[Functions]
  [./u_func]
    type   = ParsedGradFunction
    value  = 'exp(-c*(x^2+y^2))'
    grad_x = '-2*c*exp(-c*(x^2+y^2))*x'
    grad_y = '-2*c*exp(-c*(x^2+y^2))*y'
  [../]
  [./lapu_func]
    type   = ParsedFunction
    expression  = '4*c*(c*(x^2+y^2) - 1)*exp(-c*(x^2+y^2))'
  [../]
  [./forcing_func]
    type   = ParsedFunction
    expression  = '16*c^2*(c^2*(x^2+y^2)^2 - 4*c*(x^2+y^2) + 2)*exp(-c*(x^2+y^2))'
  [../]
[]

[Postprocessors]
  [./l2_error]
    type = ElementL2Error
    variable = u
    function = u_func
  [../]
  [./h1_error]
    type = ElementH1Error
    variable = u
    function = u_func
  [../]
[]

[Outputs]
  exodus = true
[]
