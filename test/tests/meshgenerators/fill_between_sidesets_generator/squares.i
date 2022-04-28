[Mesh]
  [square_1]
    type = GeneratedMeshGenerator
    nx = 3
    ny = 3
    dim = 2
    bias_y = 0.8
  []
  [rotate_1]
    type = TransformGenerator
    input = square_1
    transform = ROTATE
    vector_value = '-45 0 0'
  []
  [square_2]
    type = GeneratedMeshGenerator
    nx = 5
    ny = 5
    dim = 2
    bias_y = 1.2
  []
  [rotate_2]
    type = TransformGenerator
    input = square_2
    transform = ROTATE
    vector_value = '30 0 0'
  []
  [fbsg]
    type = FillBetweenSidesetsGenerator
    input_mesh_1 = 'rotate_1'
    input_mesh_2 = 'rotate_2'
    boundary_1 = 'top right'
    boundary_2 = 'left top'
    mesh_1_shift = '-1.5 0.5 0.0'
    mesh_2_shift = '0.8 -0.3 0.0'
    num_layers = 3
    keep_inputs = false
  []
[]
