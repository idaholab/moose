[Problem]
  solve = false
[]

[Mesh]
  # See fancy_extruder_with_boundary_swap.i for details about mesh_2d.e
  [fmg]
    type = FileMeshGenerator
    file = mesh_2d.e
    exodus_extra_element_integers = 'element_extra_integer_1 element_extra_integer_2'
  []
  [rg]
    type = RevolveGenerator
    input = fmg
    axis_point = '5.0 0.0 0.0'
    axis_direction = '0.0 1.0 0.0'
    nums_azimuthal_intervals = '2 4'
    revolving_angles = '30 60'
    elem_integer_names_to_swap = 'element_extra_integer_1 element_extra_integer_2'
    elem_integers_swaps = '1 4 2 8;
                           2 7 |
                           1 8 2 4;
                           2 5'
  []
[]

[AuxVariables]
  [element_extra_integer_1]
    family = MONOMIAL
    order = CONSTANT
  []
  [element_extra_integer_2]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [element_extra_integer_1]
    type = ExtraElementIDAux
    variable = element_extra_integer_1
    extra_id_name = element_extra_integer_1
    execute_on = 'initial'
  []
  [element_extra_integer_2]
    type = ExtraElementIDAux
    variable = element_extra_integer_2
    extra_id_name = element_extra_integer_2
    execute_on = 'initial'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
  execute_on = final
[]
