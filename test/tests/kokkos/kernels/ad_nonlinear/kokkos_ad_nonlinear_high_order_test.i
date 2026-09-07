# Nonlinear diffusion and reaction discretized with a hierarchic basis, whose polynomial order can
# be raised without changing the mesh. The natural boundary condition is used throughout so that
# every Jacobian contribution comes from the volume integral, which is where the quadrature-point
# linearization cache carries it.

[Mesh]
  [square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
    elem_type = QUAD9
  []
[]

[Variables]
  [u]
    order = THIRD
    family = HIERARCHIC
    # A nonzero state so that every block of the quadrature-point linearization is populated
    initial_condition = 1
  []
[]

[Kernels]
  [diff]
    type = KokkosADNonlinearDiffusion
    variable = u
  []
  [reaction]
    type = KokkosADNonlinearReaction
    variable = u
  []
  [source]
    type = KokkosBodyForce
    variable = u
    value = 5
  []
[]

[Executioner]
  type = Steady

  solve_type = NEWTON
[]

[Outputs]
  exodus = false
[]
