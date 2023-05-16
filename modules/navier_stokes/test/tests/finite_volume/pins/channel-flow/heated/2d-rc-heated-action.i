mu = 1
rho = 1
k = 1e-3
cp = 1
u_inlet = 1
T_inlet = 200
h_cv = 1.0

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '5 5'
    dy = '1.0'
    ix = '50 50'
    iy = '20'
    subdomain_id = '1 2'
  []
[]

[Variables]
  [T_solid]
    type = MooseVariableFVReal
  []
[]

[AuxVariables]
  [porosity]
    type = MooseVariableFVReal
    initial_condition = 0.5
  []
[]

[Modules]
  [NavierStokesFV]
    compressibility = 'incompressible'
    porous_medium_treatment = true
    add_energy_equation = true

    density = ${rho}
    dynamic_viscosity = ${mu}
    thermal_conductivity = ${k}
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

    ambient_convection_alpha = ${h_cv}
    ambient_temperature = 'T_solid'

    mass_advection_interpolation = 'average'
    momentum_advection_interpolation = 'average'
    energy_advection_interpolation = 'average'
  []
[]

[FVKernels]
  [solid_energy_diffusion]
    type = FVDiffusion
    coeff = ${k}
    variable = T_solid
  []
  [solid_energy_convection]
    type = PINSFVEnergyAmbientConvection
    variable = 'T_solid'
    is_solid = true
    T_fluid = 'T_fluid'
    T_solid = 'T_solid'
    h_solid_fluid = ${h_cv}
  []
[]

[FVBCs]
  [heated-side]
    type = FVDirichletBC
    boundary = 'top'
    variable = 'T_solid'
    value = 150
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  nl_rel_tol = 1e-14
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
