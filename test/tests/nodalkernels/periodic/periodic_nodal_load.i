# Diffusion and a reaction term on the periodic domain [0, 1] with two elements, loaded by the
# nodal load x^2 applied by a nodal kernel: 1/4 at x = 0.5 and 1 at x = 1. Periodicity constrains
# the node at x = 1 to the node at x = 0, so its load acts on the node at x = 0. With h = 1/2, the
# assembled system for the nodal values u(0) and u(0.5),
#   [[13/3, -23/6], [-23/6, 13/3]] [u(0), u(0.5)] = -[1, 1/4],
# gives u(0) = -127/98 and u(0.5) = -59/49.
# The variable w is not periodic and is loaded by u through a second nodal kernel, so the Jacobian
# row of w at x = 1 is not constrained but its column of u is. w does not couple back to u, so it
# does not change the solution for u.

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 2
  []
[]

[Variables]
  [u]
  []
  [w]
  []
[]

[AuxVariables]
  [minus_x_squared]
  []
[]

[ICs]
  [minus_x_squared]
    type = FunctionIC
    variable = minus_x_squared
    function = '-x * x'
  []
[]

[Kernels]
  [diffusion]
    type = Diffusion
    variable = u
  []
  [reaction]
    type = Reaction
    variable = u
  []
  [diffusion_w]
    type = Diffusion
    variable = w
  []
  [reaction_w]
    type = Reaction
    variable = w
  []
[]

[NodalKernels]
  [load]
    type = ADCoupledForceNodalKernel
    variable = u
    v = minus_x_squared
  []
  [coupled_load]
    type = ADCoupledForceNodalKernel
    variable = w
    v = u
  []
[]

[BCs]
  [Periodic]
    [x]
      variable = u
      auto_direction = 'x'
    []
  []
[]

[Postprocessors]
  [u_0]
    type = PointValue
    variable = u
    point = '0 0 0'
  []
  [u_half]
    type = PointValue
    variable = u
    point = '0.5 0 0'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  csv = true
[]
