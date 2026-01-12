[Mesh]
  [shape]
    type = PolyLineMeshGenerator
    points = '0.0 0.0 0.0
              1.0 1.0 0.0
              0.5 1.5 0.0
              2.0 1.5 0.0
              2.0 -1.0 0.0
              0.5 -1.0 0.0
              0.0 -2.0 0.0
              -0.5 -1.0 0.0
              -2.0 -1.0 0.0
              -2.0 1.5 0.0
              -0.5 1.5 0.0
              -1.0 1.0 0.0'
    loop = true
  []
  [xyd]
    type = XYDelaunayGenerator
    boundary = 'shape'
  []
  [gap]
    type = GapLineMeshGenerator
    input = 'xyd'
    thickness = 0.1
  []
[]
