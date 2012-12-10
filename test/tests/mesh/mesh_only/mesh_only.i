[Mesh]
  type = MeshExtruder
  file = chimney_quad.e
  num_layers = 20
  height = 1e-2
  extrusion_axis = 1 # Y
  bottom_sidesets = '2'
  top_sidesets = '4'
[]

# This input file is intended to be run with the "--mesh-only" option so
# no other sections are required
