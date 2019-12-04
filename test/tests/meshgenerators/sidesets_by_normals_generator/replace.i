[Mesh]
  # Creates (by default) the sidesets [0, 1, 2, 3]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []

  # Creates the sidesets [5, 6, 7, 8] and removes [0, 1, 2, 3]
  [generate_sidesets]
    type = AllSideSetsByNormalsGenerator
    input = gmg
    replace = true
  []
[]
