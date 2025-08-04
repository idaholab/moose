[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1'
    dy = '1 1'
    ix = '2 2'
    iy = '2 2'
    subdomain_id = '0 0
                    0 1'
  []
[]

[Adaptivity]
  adaptivity_type = p
  initial_marker = uniform
  initial_steps = 1
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

[Postprocessors]
  [avg]
    type = ElementAverageValue
    variable = u
  []
[]

[Executioner]
  type = Steady
  nl_abs_tol = 1e-10
[]

[Outputs]
  checkpoint = true
  exodus = true
  print_mesh_changed_info = true
[]
