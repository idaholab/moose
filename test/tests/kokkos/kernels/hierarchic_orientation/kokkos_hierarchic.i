# The Kokkos counterpart of hierarchic.i, solved matrix-free. Both inputs are held to the same
# result, which pins the orientation handling of the cached reference shape tables: a hierarchic
# basis of order three carries edge shape functions whose sign follows the orientation of the edge
# they belong to, so a triangular mesh presents several orientations to the tables.

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
    type = KokkosParsedFunction
    expression = '2*pi^2*sin(pi*x)*sin(pi*y)'
  []
[]

[Kernels]
  [diff]
    type = KokkosDiffusion
    variable = u
  []
  [source]
    type = KokkosBodyForce
    variable = u
    function = forcing
  []
[]

[BCs]
  [all]
    type = KokkosDirichletBC
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
  use_kokkos_matrix_free_jacobian = true
[]

[Outputs]
  file_base = hierarchic_orientation
  csv = true
[]
