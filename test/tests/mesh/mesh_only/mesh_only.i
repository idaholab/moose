[Mesh]
  uniform_refine = 1
  [file]
    type = FileMeshGenerator
    file = chimney_quad.e
  []
  [./extrude]
    input = file
    type = MeshExtruderGenerator
    num_layers = 20
    extrusion_vector = '0 1e-2 0'
    bottom_sideset = '2'
    top_sideset = '4'
  [../]
[]

[Outputs]
  [out]
    type = Exodus
    output_extra_element_ids = false
  []
[]

# This input file is intended to be run with the "--mesh-only" option so
# no other sections are required
