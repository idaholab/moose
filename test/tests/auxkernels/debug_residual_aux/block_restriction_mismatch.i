[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 1
  []
  [right_block]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    bottom_left = '0.5 0 0'
    top_right = '1 1 0'
    block_id = 1
  []
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
    block = 0
  []
[]

[AuxVariables]
  [u_residual]
    order = FIRST
    family = LAGRANGE
    block = 1
  []
[]

[Kernels]
  [diff_u]
    type = Diffusion
    variable = u
    block = 0
  []
[]

[AuxKernels]
  [debug_aux]
    type = DebugResidualAux
    debug_variable = u
    variable = u_residual
    block = 1
  []
[]

[Executioner]
  type = Steady
[]
