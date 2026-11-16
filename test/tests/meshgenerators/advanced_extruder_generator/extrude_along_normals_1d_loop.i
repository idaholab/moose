# A closed 1D loop, extruded along its node normals to grow a boundary layer around
# it. The node normals of a counter-clockwise loop point inwards, so the height is
# negative to grow the layer outwards, giving a mesh larger than the loop.
[Mesh]
  [loop]
    type = PolyLineMeshGenerator
    points = '-1 -1 0
               1 -1 0
               1  1 0
              -1  1 0'
    loop = true
  []
  [extrude]
    type = AdvancedExtruderGenerator
    input = loop
    extrude_along_node_normals = true
    heights = '-0.5'
    num_layers = '2'
  []
[]
