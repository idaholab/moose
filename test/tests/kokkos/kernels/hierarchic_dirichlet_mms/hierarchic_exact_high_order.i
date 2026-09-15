# A high-order accuracy check on the HIERARCHIC basis that does not depend on a convergence rate.
#
# The manufactured solution is a polynomial of degree eight in each variable, which lies in the space
# an order-eight HIERARCHIC basis spans on this mesh. The discretization error is therefore zero, and
# the L2 error the postprocessor reports is algebraic error alone. A rate study cannot make the same
# statement at this order: the error would collapse to roundoff after one refinement, leaving nothing
# to fit a rate to.
#
# What this covers that a low order does not: every edge of every element carries seven bubble modes
# and every element interior carries forty-nine, and the solution needs all of them. An error in a
# high-order mode's orientation, normalization, or Dirichlet projection shows up here as an L2 error
# far above the algebraic floor, whereas at order two or three each entity carries a single mode and
# most such errors cannot be expressed at all.

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
    order = EIGHTH
  []
[]

[Functions]
  # The Kokkos device dispatch and the host-side ElementL2Error postprocessor read functions from
  # separate warehouses, so the same expression is registered twice: once as a plain host Function
  # for the postprocessor, once as a Kokkos Function for the boundary condition
  [exact]
    type = ParsedFunction
    expression = 'x^8 + y^8 + x^4*y^4'
  []
  [exact_kokkos]
    type = KokkosParsedFunction
    expression = 'x^8 + y^8 + x^4*y^4'
  []
  # -laplacian of the exact solution
  [forcing]
    type = KokkosParsedFunction
    expression = '-(56*x^6 + 56*y^6 + 12*x^2*y^4 + 12*x^4*y^2)'
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

[Preconditioning]
  # An order-eight space on this mesh is too ill conditioned for the diagonal preconditioner a
  # matrix-free system defaults to: it needs thousands of linear iterations where the hierarchy needs
  # tens. Solving through the hierarchy also makes this a check on the p-multigrid solve producing the
  # right answer, rather than only on it converging.
  [pmg]
    type = PMG
    level_orders = '1 2 4'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  use_kokkos_matrix_free_jacobian = true

  # The reported L2 error is algebraic error, so it is bounded by where the solve stops rather than by
  # the basis. Both tolerances sit far below the threshold the test compares against, which keeps the
  # comparison a statement about the basis and not about the solver.
  l_tol = 1e-12
  nl_rel_tol = 1e-11
[]

[Outputs]
  csv = true
[]
