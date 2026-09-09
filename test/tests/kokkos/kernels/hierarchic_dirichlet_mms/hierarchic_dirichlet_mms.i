# A HIERARCHIC p-convergence study with a non-constant, non-preset Dirichlet Function value on
# the boundary. The manufactured solution's boundary trace varies from point to point along every
# side of the domain, so this stresses libMesh's own constraint machinery pinning a HIERARCHIC
# edge/face mode to a value the Kokkos device-side dispatch cannot reach by boundary node, both for
# the preset step and for the residual/Jacobian-vector-product/diagonal contributions a non-preset
# boundary condition needs every Newton iteration.

[Mesh]
  [square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
    # HIERARCHIC shape functions above first order require a second-order geometric element
    elem_type = QUAD9
  []
[]

[Variables]
  [u]
    family = HIERARCHIC
    order = SECOND
  []
[]

[Functions]
  # The Kokkos device dispatch and the host-side ElementL2Error postprocessor read functions from
  # separate warehouses, so the same expression is registered twice: once as a plain host Function
  # for the postprocessor, once as a Kokkos Function for the boundary condition
  [exact]
    type = ParsedFunction
    expression = 'sin(pi*x)*sin(pi*y) + x + y'
  []
  [exact_kokkos]
    type = KokkosParsedFunction
    expression = 'sin(pi*x)*sin(pi*y) + x + y'
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
    type = KokkosFunctionDirichletBC
    variable = u
    preset = false
    boundary = 'left right top bottom'
    function = exact_kokkos
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
