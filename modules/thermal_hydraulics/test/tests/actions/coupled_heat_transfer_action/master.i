# This tests an action used to exchange T_wall, T_fluid and HTC between
# a heat conduction simulation and a THM simulation

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmax = 0.1
  nx = 2
  ymax = 1
  ny = 10
  parallel_type = replicated
  coord_type = RZ
[]

[Variables]
  [T]
  []
[]

[ICs]
  [T_ic]
    type = ConstantIC
    variable = T
    value = 300
  []
[]

[AuxVariables]
  [T_fluid]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 300
  []
  [htc]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 0
  []
[]

[Kernels]
  [td]
    type = TimeDerivative
    variable = T
  []

  [diff]
    type = Diffusion
    variable = T
  []
[]

[CoupledHeatTransfers]
  [right]
    boundary = right
    T_fluid = 'T_fluid'
    T = T
    T_wall = T_wall
    htc = 'htc'
    multi_app = thm
    T_fluid_user_objects = 'T_uo'
    htc_user_objects = 'Hw_uo'

    position = '0 0 0'
    orientation = '0 1 0'
    length = 1
    n_elems = 10
    skip_coordinate_collapsing = true
  []
[]

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 10
  nl_abs_tol = 1e-10
  abort_on_solve_fail = true

  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type'
  petsc_options_value = ' lu'
[]

[MultiApps]
  [thm]
    type = TransientMultiApp
    app_type = ThermalHydraulicsApp
    input_files = sub.i
    execute_on = 'TIMESTEP_END'
    bounding_box_padding = '0.1 0 0.1'
  []
[]

[Outputs]
  exodus = true
[]

[Postprocessors]
  [T_wall_avg]
    type = SideAverageValue
    variable = T
    boundary = right
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [T_fluid_avg]
    type = ElementAverageValue
    variable = T_fluid
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [htc_avg]
    type = ElementAverageValue
    variable = htc
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]
