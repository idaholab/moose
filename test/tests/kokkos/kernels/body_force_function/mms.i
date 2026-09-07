# A manufactured solution driven by a spatially varying body force. The forcing is the Laplacian of
# the exact solution, so the discrete solution converges to it at the rate of the basis, and the L2
# error is what the test records.

[Mesh]
  [square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 8
    ny = 8
  []
[]

[Variables]
  [u]
    order = FIRST
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
  csv = true
[]
