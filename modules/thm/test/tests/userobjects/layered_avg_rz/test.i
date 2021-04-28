length = 4

[GlobalParams]
[]

[UserObjects]
  [average_temp_uo]
    type = LayeredAverageRZ
    execute_on = 'initial timestep_end'
    direction = z
    variable = T_solid
    block = hs:1
    num_layers = 10
    axis_point = '0 0 0'
    axis_dir = '0 0 1'
    length = ${length}
  []
[]

[AuxVariables]
  [average_temp]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [layered_average]
    type = SpatialUserObjectAux
    variable = average_temp
    execute_on = 'initial timestep_end'
    user_object = average_temp_uo
  []
[]

[HeatStructureMaterials]
  [mat1]
    type = SolidMaterialProperties
    k = 2.5
    cp = 300.
    rho = 1.032e4
  []
  [mat2]
    type = SolidMaterialProperties
    k = 0.6
    cp = 1.
    rho = 1.
  []
  [mat3]
    type = SolidMaterialProperties
    k = 21.5
    cp = 350.
    rho = 6.55e3
  []
[]

[Components]
  [hs]
    type = HeatStructureCylindrical
    position = '0 0 0'
    orientation = '0 0 1'
    length = ${length}
    n_elems = 20

    initial_T = '300 + 10 * sin(0.5 * z * pi / 3.865)'

    names = '1 2 3'
    widths = '0.004 0.0001 0.0005'
    n_part_elems = '10 1 2'
    materials = 'mat1 mat2 mat3'
  []
[]

[Preconditioning]
  [pc]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  start_time = 0
  dt = 0.5
  num_steps = 1
  abort_on_solve_fail = true

  solve_type = 'PJFNK'
  line_search = 'basic'
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-9
  nl_max_its = 10

  l_tol = 1e-3
  l_max_its = 100
[]

[Outputs]
  exodus = true
  show = 'average_temp'
[]
