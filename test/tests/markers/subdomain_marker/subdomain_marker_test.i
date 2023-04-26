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

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Adaptivity]
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
