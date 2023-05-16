# Solid properties
cp_s = 2
rho_s = 4
k_s = 1e-2
h_fs = 10

# Operating conditions
u_inlet = 1
T_inlet = 200
p_outlet = 10
top_side_temperature = 150

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 10
    ymin = 0
    ymax = 1
    nx = 20
    ny = 5
  []
[]

[Variables]
  [T_solid]
    type = MooseVariableFVReal
    initial_condition = 100
  []
[]

[AuxVariables]
  [porosity]
    type = MooseVariableFVReal
    initial_condition = 0.5
  []
  [velocity_norm]
    type = MooseVariableFVReal
  []
[]

[FluidProperties]
  [fp]
    type = FlibeFluidProperties
  []
[]

[Modules]
  [NavierStokesFV]
    compressibility = 'weakly-compressible'
    add_energy_equation = true
    porous_medium_treatment = true

    density = 'rho'
    dynamic_viscosity = 'mu'
    thermal_conductivity = 'k'
    specific_heat = 'cp'

    initial_velocity = '${u_inlet} 1e-6 0'
    initial_pressure = '${p_outlet}'
    initial_temperature = '${T_inlet}'

    inlet_boundaries = 'left'
    momentum_inlet_types = 'fixed-velocity'
    momentum_inlet_function = '${u_inlet} 0'
    energy_inlet_types = 'fixed-temperature'
    energy_inlet_function = '${T_inlet}'

    wall_boundaries = 'top bottom'
    momentum_wall_types = 'noslip symmetry'
    energy_wall_types = 'heatflux heatflux'
    energy_wall_function = '0 0'

    outlet_boundaries = 'right'
    momentum_outlet_types = 'fixed-pressure'
    pressure_function = '${p_outlet}'

    ambient_convection_alpha = 'h_cv'
    ambient_temperature = 'T_solid'

    mass_advection_interpolation = 'average'
    momentum_advection_interpolation = 'average'
    energy_advection_interpolation = 'average'
  []
[]

[FVKernels]
  [solid_energy_time]
    type = PINSFVEnergyTimeDerivative
    variable = T_solid
    cp = ${cp_s}
    rho = ${rho_s}
    is_solid = true
    porosity = 'porosity'
  []
  [solid_energy_diffusion]
    type = FVDiffusion
    variable = T_solid
    coeff = ${k_s}
  []
  [solid_energy_convection]
    type = PINSFVEnergyAmbientConvection
    variable = T_solid
    is_solid = true
    T_fluid = 'T_fluid'
    T_solid = 'T_solid'
    h_solid_fluid = 'h_cv'
  []
[]

[FVBCs]
  [heated-side]
    type = FVDirichletBC
    boundary = 'top'
    variable = 'T_solid'
    value = ${top_side_temperature}
  []
[]

[Materials]
  [const_functor]
    type = ADGenericFunctorMaterial
    prop_names = 'h_cv'
    prop_values = '${h_fs}'
  []
  [fluid_props_to_mat_props]
    type = GeneralFunctorFluidProps
    fp = fp
    pressure = 'pressure'
    T_fluid = 'T_fluid'
    speed = 'velocity_norm'

    # To initialize with a high viscosity
    mu_rampdown = 'mu_rampdown'

    # For porous flow
    characteristic_length = 1
    porosity = 'porosity'
  []
[]

[Functions]
  [mu_rampdown]
    type = PiecewiseLinear
    x = '1 2 3 4'
    y = '1e3 1e2 1e1 1'
  []
[]

[AuxKernels]
  [speed]
    type = ParsedAux
    variable = 'velocity_norm'
    coupled_variables = 'superficial_vel_x superficial_vel_y porosity'
    expression = 'sqrt(superficial_vel_x*superficial_vel_x + superficial_vel_y*superficial_vel_y) / '
               'porosity'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_pc_type -sub_pc_factor_shift_type'
  petsc_options_value = 'asm      100                lu           NONZERO'
  line_search = 'none'
  nl_rel_tol = 1e-12

  end_time = 3.0
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
