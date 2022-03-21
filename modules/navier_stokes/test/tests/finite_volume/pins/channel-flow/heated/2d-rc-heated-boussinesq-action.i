mu=1
rho=1
k=1e-3
cp=1
v_inlet=1
T_inlet=200

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 2
    ymin = 0
    ymax = 10
    nx = 20
    ny = 100
  []
[]

[AuxVariables]
  [T_solid]
    type = MooseVariableFVReal
    initial_condition = 100
  []
  [porosity]
    type = MooseVariableFVReal
    initial_condition = 0.4
  []
[]

[Modules]
  [NavierStokesFV]
    simulation_type = 'steady-state'
    compressibility = 'incompressible'
    porous_medium_treatment = true
    add_energy_equation = true
    boussinesq_approximation = true

    density = 'rho'
    dynamic_viscosity = 'mu'
    thermal_conductivity = 'k'
    specific_heat = 'cp'
    porosity = 'porosity'
    thermal_expansion = 'alpha_b'
    gravity = '0 -9.81 0'
    ref_temperature = 150

    initial_velocity = '1e-6 ${v_inlet} 0'
    initial_pressure = 0.0
    initial_temperature = 0.0

    inlet_boundaries = 'bottom'
    momentum_inlet_types = 'fixed-velocity'
    momentum_inlet_function = '0 ${v_inlet}'
    energy_inlet_types = 'heatflux'
    energy_inlet_function = '${fparse v_inlet * rho * cp * T_inlet}'

    wall_boundaries = 'right left'
    momentum_wall_types = 'noslip symmetry'
    energy_wall_types = 'heatflux heatflux'
    energy_wall_function = '0 0'

    outlet_boundaries = 'top'
    momentum_outlet_types = 'fixed-pressure'
    pressure_function = '0'

    ambient_convection_alpha = 'h_cv'
    ambient_temperature = 'T_solid'
  []
[]

[Materials]
  [constants]
    type = ADGenericFunctorMaterial
    prop_names = 'h_cv alpha_b cp k rho mu'
    prop_values = '1e-3 8e-4 ${cp} ${k} ${rho} ${mu}'
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

# Some basic Postprocessors to examine the solution
[Postprocessors]
  [inlet-p]
    type = SideAverageValue
    variable = pressure
    boundary = 'top'
  []
  [outlet-v]
    type = SideAverageValue
    variable = superficial_vel_y
    boundary = 'top'
  []
  [outlet-temp]
    type = SideAverageValue
    variable = T_fluid
    boundary = 'top'
  []
[]

[Outputs]
  exodus = true
  csv = false
[]
