# A hierarchic basis of order three on a triangular mesh, whose edge shape functions depend on the
# orientation of the edge they belong to. The Kokkos companion input solves the same problem and is
# held to the same result, which is what pins the orientation handling of the cached reference shape
# tables.

[Mesh]
  [square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
    elem_type = TRI6
  []
[]

[Variables]
  [u]
    family = HIERARCHIC
    order = THIRD
  []
[]

[Functions]
  [exact]
    type = ParsedFunction
    expression = 'sin(pi*x)*sin(pi*y)'
  []
  [forcing]
    type = ParsedFunction
    expression = '2*pi^2*sin(pi*x)*sin(pi*y)'
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [source]
    type = BodyForce
    variable = u
    function = forcing
  []
[]

[BCs]
  [all]
    type = DirichletBC
    variable = u
    preset = false
    boundary = 'left right top bottom'
    value = 0
  []
[]

[Postprocessors]
  [l2_error]
    type = ElementL2Error
    variable = u
    function = exact
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  file_base = hierarchic_orientation
  csv = true
[]
