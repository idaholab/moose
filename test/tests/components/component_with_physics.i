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

[Physics]
  [Diffusion]
    [FiniteVolume]
      [block_specified]
        diffusivity_functor = 1
        block = '2 cyl1'
        #this BC gets added on all three meshes because the 'left' is shared
        #This is something we will want component to prevent
        dirichlet_boundaries = 'left cylinder_1_left'
        boundary_values = '3 1'
        source_functor = '1'
        source_coef = '1'
      []
      [added_from_component]
        variable_name = v
        diffusivity_functor = 2
        source_functor = '2'
        source_coef = '1'
        dirichlet_boundaries = 'cylinder_2_right'
        boundary_values = '2'

      []
    []
  []
[]

[Components]

  [cylinder_1]
    type = Cylinder
    dimension = 2
    radius = 2
    height = 10
    position = '1 0 0'
    direction = '0 1 0'
    block = 'cyl1'
  []
  [cylinder_2]
    type = Cylinder
    dimension = 2
    radius = 4
    height = 1
    position = '2 0 0'
    direction = '0 0 1'
    physics = 'added_from_component'
    block = cyl2
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
