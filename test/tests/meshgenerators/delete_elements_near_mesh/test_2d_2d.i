[Mesh]
  [to_be_cut]
    type = CartesianMeshGenerator
    dim = 2
    dx = '6'
    dy = '6'
    ix = '6'
    iy = '6'
  []
  [to_cut_with]
    type = CartesianMeshGenerator
    dim = 2
    dx = '${fparse sqrt(2) * 4}'
    dy = '0.3'
    ix = '2'
    iy = '1'
  []
  [rotate]
    type = TransformGenerator
    input = 'to_cut_with'
    transform = 'ROTATE'
    vector_value = '45 0 0'
  []
  [delete]
    type = DeleteElementsNearMeshGenerator
    input = 'to_be_cut'
    distance = 0.1
    proximity_mesh = 'rotate'
    side_order = CONSTANT
  []
[]
