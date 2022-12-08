[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 4
    ymin = -1
    ymax = 1
    nx = 4
    ny = 2
  []
  [right]
    type = ParsedSubdomainMeshGenerator
    input = gen
    combinatorial_geometry = 'x > 2'
    block_id = 1
  []
  [left]
    type = ParsedSubdomainMeshGenerator
    input = right
    combinatorial_geometry = 'x < 2'
    block_id = 2
  []
[]

[Modules]
  [NavierStokesFV]
    compressibility = 'incompressible'
    add_energy_equation = true

    density = 1
    dynamic_viscosity = 1
    thermal_conductivity = 1e-3
    specific_heat = 1

    initial_velocity = '1 1 0'
    initial_pressure = 0.0
    initial_temperature = 0.0

    inlet_boundaries = 'left'
    momentum_inlet_types = 'fixed-velocity'
    momentum_inlet_function = '1 0'
    energy_inlet_types = 'fixed-temperature'
    energy_inlet_function = '1'

    wall_boundaries = 'top bottom'
    momentum_wall_types = 'noslip noslip'
    energy_wall_types = 'heatflux heatflux'
    energy_wall_function = '0 0'

    outlet_boundaries = 'right'
    momentum_outlet_types = 'fixed-pressure'
    pressure_function = '0'

    ambient_convection_alpha = 1
    ambient_temperature = '100'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_pc_type -sub_pc_factor_shift_type'
  petsc_options_value = 'asm      100                lu           NONZERO'
  line_search = 'none'
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
