[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 20
    ny = 10
    xmax = 10
    ymax = 3
    elem_type = TRI3
  []
  [pin]
    type = ExtraNodesetGenerator
    nodes = 106
    new_boundary = pin
    input = gen
  []
  displacements = 'disp_x disp_y'
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
[]

[BCs]
  [left_x]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  []
  [left_y]
    type = DirichletBC
    variable = disp_y
    boundary = pin
    value = 0
  []

  [right_neumann]
    type = FunctionNeumannBC
    variable = disp_x
    function = t
    boundary = right
  []
[]

[UserObjects]
  [uel]
    type = AbaqusUserElement
    variables = 'disp_x disp_y'
    plugin = ../../plugins/small_strain_tri_energy_uel
    use_displaced_mesh = false
    num_state_vars = 8
    use_energy = true
    constant_properties = '100 0.3' # E nu
  []
[]

[Problem]
  kernel_coverage_check = false
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  dt = 1
  num_steps = 3
[]

[Postprocessors]
  [delta_l]
    type = SideAverageValue
    variable = disp_x
    boundary = right
  []
  [strain_energy]
    type = AbaqusTotalUELEnergy
    uel_user_object = uel
    component = 1
  []
  # Clapeyron: for a linear elastic body the strain energy is half the work of the applied
  # traction, here (1/2) * t * ymax * delta_l with traction t and ymax = 3
  [half_work]
    type = ParsedPostprocessor
    expression = '0.5 * t * 3 * delta_l'
    pp_names = 'delta_l'
    use_t = true
  []
[]

[Outputs]
  csv = true
[]
