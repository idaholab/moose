[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    ix = 2
    iy = 2
    dx = 1
    dy = 1
    
  []

  final_generator = 'cmg'
 
[]

[Components]
  [cylinder_1]
    type = Cylinder
    dimension = 2
    radius = 2
    height = 10
    position = '1 0 0'
    direction = '0 1 0'
  []
  [cylinder_2]
    type = Cylinder
    dimension = 2
    radius = 4
    height = 1
    position = '2 0 0'
    direction = '0 0 1'
  []
[]

[Problem]
  skip_nl_system_check = true
[]

[Executioner]
  type = Steady
[]
