mu = 1
rho = 1
k = 1e-3
cp = 1
alpha = 1

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 5
    ymin = -1
    ymax = 1
    nx = 10
    ny = 4
  []
  [right]
    type = ParsedSubdomainMeshGenerator
    input = gen
    combinatorial_geometry = 'x > 2.5'
    block_id = 1
  []
  [left]
    type = ParsedSubdomainMeshGenerator
    input = right
    combinatorial_geometry = 'x < 2.5'
    block_id = 2
  []
[]

[Variables]
  inactive = 'vel_x vel_y pressure T_fluid scalar'
  [vel_x]
    type = 'INSFVVelocityVariable'
    initial_condition = 1
  []
  [vel_y]
    type = 'INSFVVelocityVariable'
    initial_condition = 1
  []
  [pressure]
    type = 'INSFVPressureVariable'
    initial_condition = 0
  []
  [T_fluid]
    type = 'INSFVEnergyVariable'
    initial_condition = 0
  []
  [scalar]
    type = MooseVariableFVReal
    initial_condition = 0
  []
[]

[Modules]
  [NavierStokesFV]
    compressibility = 'incompressible'
    porous_medium_treatment = false
    add_energy_equation = true
    add_scalar_equation = true

    passive_scalar_names = 'scalar'

    density = 'rho'
    dynamic_viscosity = 'mu'
    thermal_conductivity = 'k'
    specific_heat = 'cp'

    passive_scalar_diffusivity = 1e-3
    passive_scalar_source = 0.1

    initial_velocity = '1 1 0'
    initial_pressure = 0.0
    initial_temperature = 0.0

    inlet_boundaries = 'left'
    momentum_inlet_types = 'fixed-velocity'
    momentum_inlet_functors = '1 0'
    energy_inlet_types = 'fixed-temperature'
    energy_inlet_functors = '1'
    passive_scalar_inlet_types = 'fixed-value'
    passive_scalar_inlet_functors = '1'

    wall_boundaries = 'top bottom'
    momentum_wall_types = 'noslip noslip'
    energy_wall_types = 'heatflux heatflux'
    energy_wall_functors = '0 0'

    outlet_boundaries = 'right'
    momentum_outlet_types = 'fixed-pressure'
    pressure_functors = '0'

    ambient_convection_alpha = 'alpha'
    ambient_temperature = '100'

    friction_blocks = '1; 2'
    friction_types = 'Darcy; Darcy'
    friction_coeffs = 'friction_coefficient; friction_coefficient'

    standard_friction_formulation = false
  []
[]

[FunctorMaterials]
  [const_functor]
    type = ADGenericFunctorMaterial
    prop_names = 'cp k rho mu alpha'
    prop_values = '${cp} ${k} ${rho} ${mu} ${alpha}'
  []
  [kappa]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'kappa'
    prop_values = '1 1 1'
  []
  [friction_coefficient]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'friction_coefficient'
    prop_values = '1 1 1'
  []
[]

[Postprocessors]
  [temp]
    type = ElementAverageValue
    variable = T_fluid
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
