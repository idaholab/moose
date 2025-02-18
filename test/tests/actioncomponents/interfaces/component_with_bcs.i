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
    [ContinuousGalerkin]
      [diff_u]
        diffusivity_matprop = 'diff'
        block = '2 cyl1 cyl2'

        # This boundary condition is applied on the cmg-generated mesh
        dirichlet_boundaries = 'left'
        boundary_values = '1'

        source_functor = '1'
        source_coef = '1'
      []
      [diff_v]
        variable_name = v
        diffusivity_matprop = 'diff'
        source_functor = '2'
        source_coef = '1'
      []
    []
  []
[]

[ActionComponents]
  [cylinder_1]
    type = CylinderComponent

    # Geometry parameter
    dimension = 2
    radius = 2
    length = 10
    n_axial = 1
    n_radial = 1
    position = '1 0 0'
    direction = '0 1 0'
    block = 'cyl1'

    physics = 'diff_u'

    initial_condition_variables = 'u'
    initial_condition_values = '1.1'

    fixed_value_bc_variables = 'u'
    fixed_value_bc_boundaries = 'cylinder_1_left'
    fixed_value_bc_values = '3'

    property_names = 'diff'
    property_values = '1'
  []
  [cylinder_2]
    type = CylinderComponent

    # Geometry parameter
    dimension = 2
    radius = 4
    length = 1
    n_axial = 1
    n_radial = 1
    position = '2 0 0'
    direction = '0 0 1'
    block = 'cyl2'

    physics = 'diff_v diff_u'

    initial_condition_variables = 'u v'
    initial_condition_values = '1.2 1.3'

    fixed_value_bc_variables = 'v u'
    fixed_value_bc_boundaries = 'cylinder_2_right; cylinder_2_left'
    fixed_value_bc_values = '2; 1'

    flux_bc_variables = 'u'
    flux_bc_boundaries = 'cylinder_2_right'
    flux_bc_values = '4'

    property_names = 'diff'
    property_values = '2'
  []
[]

[Materials]
  [on_mesh_generator_block]
    type = ADGenericConstantMaterial
    prop_names = 'diff'
    prop_values = '1'
    block = '2'
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
