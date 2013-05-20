[Mesh]
  type = FileMesh
  file = chimney_quad.e
  uniform_refine = 1
[]

[MeshModifiers]
  [./extrude]
    type = MeshExtruder
    num_layers = 20
    extrusion_vector = '0 1e-2 0'
    bottom_sideset = '2'
    top_sideset = '4'
  [../]
[]

# This input file is intended to be run with the "--mesh-only" option so
# no other sections are required
