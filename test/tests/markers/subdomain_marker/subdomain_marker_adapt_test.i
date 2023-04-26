[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
    subdomain_ids = '0 0 0 0 0
                     0 1 1 0 0
                     0 0 1 1 0
                     1 0 0 1 0
                     0 0 0 0 0'
  []
[]

[Variables]
  [u][]
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
  [top]
    type = DirichletBC
    variable = u
    boundary = top
    value = 0
  []
[]

[Executioner]
  type = Steady
[]

[Adaptivity]
  steps = 1
  marker = subdomain
  [Markers]
    [subdomain]
      type = SubdomainMarker
      block = 1
      inside = refine
      outside = do_nothing
    []
  []
[]

[Outputs]
  exodus = true
[]
