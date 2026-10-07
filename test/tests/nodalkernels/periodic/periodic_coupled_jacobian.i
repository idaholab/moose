# Only v is periodic, so the degree of freedom of v at x = 1 is constrained to the one at x = 0.
# The nodal kernel on u couples to v, so the Jacobian row of u at x = 1 is not constrained but its
# column of v is.

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
  [v]
  []
[]

[Kernels]
  [diffusion_u]
    type = Diffusion
    variable = u
  []
  [reaction_u]
    type = Reaction
    variable = u
  []
  [diffusion_v]
    type = Diffusion
    variable = v
  []
  [reaction_v]
    type = Reaction
    variable = v
  []
  [source_v]
    type = BodyForce
    variable = v
  []
[]

[NodalKernels]
  [coupled_force]
    type = ADCoupledForceNodalKernel
    variable = u
    v = v
  []
[]

[BCs]
  [Periodic]
    [x]
      variable = v
      auto_direction = 'x'
    []
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]
