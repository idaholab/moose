# Reference for biharmonic_ad_array.i with one standard HERMITE variable per component
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 6
  ny = 6
  xmax = 2
  ymax = 2
[]

[Variables]
  [u0]
    order = THIRD
    family = HERMITE
  []
  [u1]
    order = THIRD
    family = HERMITE
  []
[]

[Kernels]
  [biharmonic0]
    type = Biharmonic
    variable = u0
  []
  [biharmonic1]
    type = Biharmonic
    variable = u1
  []
  [reaction0]
    type = ADReaction
    variable = u0
  []
  [reaction1]
    type = ADReaction
    variable = u1
  []
  [force0]
    type = BodyForce
    variable = u0
    function = f0
  []
  [force1]
    type = BodyForce
    variable = u1
    function = f1
  []
[]

[Functions]
  # u0 = sin(pi x) cos(pi y), u1 = cos(pi x); laplacian^2 of sin/cos(pi x) products gives the
  # factors (2 pi^2)^2 and pi^4
  [u0]
    type = ParsedFunction
    expression = 'sin(pi*x)*cos(pi*y)'
  []
  [u1]
    type = ParsedFunction
    expression = 'cos(pi*x)'
  []
  [f0]
    type = ParsedFunction
    expression = '(4*pi^4 + 1)*sin(pi*x)*cos(pi*y)'
  []
  [f1]
    type = ParsedFunction
    expression = '(pi^4 + 1)*cos(pi*x)'
  []
[]

[BCs]
  [Periodic]
    [all]
      auto_direction = 'x y'
      variable = 'u0 u1'
    []
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  nl_rel_tol = 1e-12
  [Quadrature]
    type = GAUSS
    order = ELEVENTH
  []
[]

[Postprocessors]
  [norm0]
    type = ElementL2Norm
    variable = u0
  []
  [norm1]
    type = ElementL2Norm
    variable = u1
  []
  [error0]
    type = ElementL2Error
    variable = u0
    function = u0
    outputs = console
  []
  [error1]
    type = ElementL2Error
    variable = u1
    function = u1
    outputs = console
  []
[]

[Outputs]
  file_base = biharmonic_out
  csv = true
[]
