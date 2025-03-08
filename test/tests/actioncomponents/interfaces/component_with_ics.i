[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    ix = 2
    iy = 2
    dx = 1
    dy = 1
    subdomain_id = 2
  []
  final_generator = 'cmg'
[]

[Physics]
  [Diffusion]
    [FiniteVolume]
      [phy1]
        diffusivity_functor = 'diff'
        # we make this physics defined everywhere to have a vector input of initial conditions
        block = '2'
        dirichlet_boundaries = 'left cylinder_1_left cylinder_2_right'
        boundary_values = '1 3 2'
        source_functor = '1'
        source_coef = '1'
      []
      [phy2]
        variable_name = v
        diffusivity_functor = 'diff'
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
    physics = 'phy1'
    block = 'cyl1'

    initial_condition_variables = 'u'
    initial_condition_values = '1.1'
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
    physics = 'phy1 phy2'
    block = 'cyl2'

    initial_condition_variables = 'u v'
    initial_condition_values = '1.2 1.3'
  []
[]

[FunctorMaterials]
  [diff]
    type = ADPiecewiseByBlockFunctorMaterial
    prop_name = 'diff'
    subdomain_to_prop_value = 'cyl1 1
                               2 1
                               cyl2 2'
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [min_u_c1]
    type = ElementExtremeValue
    value_type = min
    variable = u
    block = 'cyl1'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [min_u_c2]
    type = ElementExtremeValue
    value_type = min
    variable = u
    block = 'cyl2'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [min_v]
    type = ElementExtremeValue
    value_type = min
    variable = v
    block = 'cyl2'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [max_u_c1]
    type = ElementExtremeValue
    variable = u
    block = 'cyl1'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [max_u_c2]
    type = ElementExtremeValue
    variable = u
    block = 'cyl2'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [max_v]
    type = ElementExtremeValue
    variable = v
    block = 'cyl2'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [ave_u_c1]
    type = ElementAverageValue
    variable = u
    block = 'cyl1'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [ave_u_c2]
    type = ElementAverageValue
    variable = u
    block = 'cyl2'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [ave_v]
    type = ElementAverageValue
    variable = v
    block = 'cyl2'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Outputs]
  csv = true
[]
