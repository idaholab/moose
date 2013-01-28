[Mesh]
  type = MeshExtruder
  file = chimney_quad.e
  num_layers = 20
  extrusion_vector = '0 1e-2 0'
  bottom_sideset = '2'
  top_sideset = '4'
[]

# This input file is intended to be run with the "--mesh-only" option so
# no other sections are required
