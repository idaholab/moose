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
        dirichlet_boundaries = 'left cylinder_1_left'
        boundary_values = '1 3'
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

[ActionComponents]
  [cylinder_1]
    type = CylinderComponent
    dimension = 2
    radius = 2
    length = 10
    n_axial = 1
    n_radial = 1
    position = '1 0 0'
    direction = '0 1 0'
    block = 'cyl1'
  []
  [cylinder_2]
    type = CylinderComponent
    dimension = 2
    radius = 4
    length = 1
    n_axial = 1
    n_radial = 1
    position = '2 0 0'
    direction = '0 0 1'
    physics = 'added_from_component'
    block = 'cyl2'
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [min_u]
    type = ElementExtremeValue
    value_type = min
    variable = u
    block = '2 cyl1'
  []
  [min_v]
    type = ElementExtremeValue
    value_type = min
    variable = v
    block = 'cyl2'
  []
  [max_u]
    type = ElementExtremeValue
    variable = u
    block = '2 cyl1'
  []
  [max_v]
    type = ElementExtremeValue
    variable = v
    block = 'cyl2'
  []
  [ave_u]
    type = ElementAverageValue
    variable = u
    block = '2 cyl1'
  []
  [ave_v]
    type = ElementAverageValue
    variable = v
    block = 'cyl2'
  []
[]

[Outputs]
  csv = true
[]
