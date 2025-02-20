[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
  []
[]

[Variables]
  [u]
    type = MooseVariableFVReal
  []
[]

[FVKernels]
  [diffusion]
    type = FVDiffusion
    coeff = 1
    variable = u
  []
  [source]
    type = FVBodyForce
    variable = u
    value = 1
  []
  # We don't add matrix entries for the aggregate-based BC so add this to make the PC nonsingular
  [rxn]
    type = FVReaction
    variable = u
  []
[]

[FVBCs]
  [flux_out]
    type = FVFunctorNeumannBC
    boundary = 'left right'
    functor = layered_average
    variable = u
    factor = '-1'
  []
[]

[UserObjects]
  [layered_average]
    execute_on = 'linear nonlinear'
    type = LayeredAverage
    direction = 'y'
    variable = u
    num_layers = 5
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
[]

[Outputs]
  exodus = true
[]
