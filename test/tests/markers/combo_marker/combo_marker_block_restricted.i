[Mesh]
  [base]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    elem_type = QUAD4
  []
  [subdomain_1]
    type = SubdomainBoundingBoxGenerator
    input = base
    bottom_left = '0.1 0.1 0'
    block_id = 1
    top_right = '0.4 0.9 0'
  []
  [subdomain_2]
    type = SubdomainBoundingBoxGenerator
    input = subdomain_1
    bottom_left = '0.1 0.1 0'
    block_id = 2
    top_right = '0.4 0.9 0'
    location = OUTSIDE
  []
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  []
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Adaptivity]
  initial_marker = combo
  initial_steps = 1
  [Markers]
    [box]
      type = BoxMarker
      bottom_left = '0.0 0. 0'
      top_right = '1 0.5 0'
      inside = refine
      outside = do_nothing
      block = '1'
    []
    [box2]
      type = BoxMarker
      bottom_left = '0.5 0.5 0'
      top_right = '0.8 0.8 0'
      inside = refine
      outside = coarsen
      block = '2'
    []
    [combo]
      type = ComboMarker
      markers = 'box box2'
    []
  []
[]

[Outputs]
  exodus = true
[]
