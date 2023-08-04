[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1'
    dy = '1 1'
    ix = '5 5'
    iy = '5 5'
    subdomain_id = '0 0
                    0 1'
  []
[]

[Adaptivity]
  switch_h_to_p_refinement = true
  initial_marker = uniform
  initial_steps = 1
  disable_lagrange_p_refinement = true
  [Markers/uniform]
    type = UniformMarker
    mark = REFINE
    block = 1
  []
[]

[Variables]
  [u]
    family = MONOMIAL
    order = FIRST
  []
[]

[AuxVariables]
  [test][]
[]

[ICs]
  [test]
    variable = test
    type = FunctionIC
    function = 'x + y'
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [src]
    type = BodyForce
    variable = u
    value = 1
  []
[]

[DGKernels]
  [dg_diff]
    type = DGDiffusion
    variable = u
    epsilon = -1
    sigma = 6
  []
[]

[BCs]
  [left_u]
    type = DGFunctionDiffusionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = 0
    epsilon = -1
    sigma = 6
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
