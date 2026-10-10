# Periodic biharmonic problem with a reaction term, laplacian(laplacian(u)) + u = f, on HERMITE
# elements for a two-component array variable. biharmonic_reference.i solves the same problem
# with standard variables; both write the same component L2 norms to one gold file.
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 6
  ny = 6
  xmax = 2
  ymax = 2
[]

[Variables]
  [u]
    order = THIRD
    family = HERMITE
    components = 2
  []
[]

[Kernels]
  [biharmonic]
    type = ADArrayBiharmonic
    variable = u
  []
  [reaction]
    type = ADArrayReaction
    variable = u
    reaction_coefficient = one
  []
  [force]
    type = ArrayBodyForce
    variable = u
    function = 'f0 f1'
  []
[]

[Materials]
  [one]
    type = ADGenericConstant2DArray
    prop_name = one
    prop_value = '1 0; 0 1'
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
      variable = u
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
    type = ElementArrayL2Norm
    variable = u
    component = 0
  []
  [norm1]
    type = ElementArrayL2Norm
    variable = u
    component = 1
  []
[]

[Outputs]
  file_base = biharmonic_out
  csv = true
[]
