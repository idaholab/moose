mu = 1
rho = 1
cp = 1
u_inlet = 1
T_inlet = 200

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 10
    ymin = 0
    ymax = 1
    nx = 100
    ny = 20
  []
  [left]
    type = ParsedSubdomainMeshGenerator
    input = gen
    combinatorial_geometry = 'x > 3 & x < 6'
    block_id = 1
  []
  [right]
    type = ParsedSubdomainMeshGenerator
    input = left
    combinatorial_geometry = 'x < 3'
    block_id = 2
  []
  [more-right]
    type = ParsedSubdomainMeshGenerator
    input = right
    combinatorial_geometry = 'x > 6'
    block_id = 3
  []
[]

[AuxVariables]
  [porosity]
    type = MooseVariableFVReal
    initial_condition = 0.5
  []
  [T_solid]
    family = 'MONOMIAL'
    order = 'CONSTANT'
    fv = true
    initial_condition = 100
  []
[]

[Modules]
  [NavierStokesFV]
    compressibility = 'incompressible'
    porous_medium_treatment = true
    add_energy_equation = true

    density = ${rho}
    dynamic_viscosity = ${mu}
    thermal_conductivity_blocks = '1 2; 3'
    thermal_conductivity = 'kappa kappa'
    specific_heat = ${cp}
    porosity = 'porosity'

    initial_velocity = '${u_inlet} 1e-6 0'
    initial_pressure = 0.0
    initial_temperature = 0.0

    inlet_boundaries = 'left'
    momentum_inlet_types = 'fixed-velocity'
    momentum_inlet_function = '${u_inlet} 0'
    energy_inlet_types = 'heatflux'
    energy_inlet_function = '${fparse u_inlet * rho * cp * T_inlet}'

    wall_boundaries = 'top bottom'
    momentum_wall_types = 'noslip symmetry'
    energy_wall_types = 'heatflux heatflux'
    energy_wall_function = '0 0'

    outlet_boundaries = 'right'
    momentum_outlet_types = 'fixed-pressure'
    pressure_function = '0.1'

    ambient_convection_alpha = 'h_cv'
    ambient_temperature = 'T_solid'

    mass_advection_interpolation = 'average'
    momentum_advection_interpolation = 'average'
    energy_advection_interpolation = 'average'
  []
[]

[Materials]
  [kappa]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'kappa'
    prop_values = '1e-3 1e-2 1e-1'
  []
  [constants]
    type = ADGenericFunctorMaterial
    prop_names = 'h_cv'
    prop_values = '1'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  nl_rel_tol = 1e-12
[]

# Some basic Postprocessors to examine the solution
[Postprocessors]
  [inlet-p]
    type = SideAverageValue
    variable = pressure
    boundary = 'left'
  []
  [outlet-u]
    type = SideAverageValue
    variable = superficial_vel_x
    boundary = 'right'
  []
  [outlet-temp]
    type = SideAverageValue
    variable = T_fluid
    boundary = 'right'
  []
  [solid-temp]
    type = ElementAverageValue
    variable = T_solid
  []
[]

[Outputs]
  exodus = true
  csv = false
[]
