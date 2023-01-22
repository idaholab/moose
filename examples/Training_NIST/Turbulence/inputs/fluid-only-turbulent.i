################################################################################
# Incompressible turbulent fluid flow for standalone simulations
################################################################################

################################################################################
# WARNING: You need to increase the derivative container size for this
# Recommended value: 200
# Use ./configure --with-derivative-size=200 command in the moose root directory
################################################################################

################################################################################
# Parameters for boundary/initial conditions and material properties
################################################################################
u_inlet = 5.0 # m/s
p_outlet = 1.2e6 # Pa
T_inlet = 300 # K
kappa_VK = 0.41 # von Kármán constant

[Modules]
  [NavierStokesFV]
    compressibility = 'incompressible'
    block = 'water'

    density = 'rho'
    dynamic_viscosity = 'mu'

    initial_velocity = '0 0 ${u_inlet}'
    initial_pressure = ${p_outlet}

    turbulence_handling = mixing-length
    mixing_length_delta = 0.005
    von_karman_const = '${fparse kappa_VK}'
    mixing_length_walls = 'clad_wall assembly_wall'

    inlet_boundaries = 'fluid-inlet'
    momentum_inlet_types = 'fixed-velocity'
    momentum_inlet_function = '0 0 ${u_inlet}'

    wall_boundaries = 'clad_wall assembly_wall left right'
    momentum_wall_types = 'noslip noslip symmetry symmetry '

    outlet_boundaries = 'fluid-outlet'
    momentum_outlet_types = 'fixed-pressure'
    pressure_function = '${p_outlet}'

    mass_advection_interpolation = 'upwind'
    momentum_advection_interpolation = 'upwind'

    gravity = '0 0 -9.81'
  []
[]

[FluidProperties]
  [water_properties]
    type = Water97FluidProperties
  []
[]

[Materials]
  [fluid_props]
    type = GeneralFunctorFluidProps
    fp = 'water_properties'
    characteristic_length = 1
    porosity = 1
    pressure = pressure
    speed = speed
    T_fluid = ${T_inlet}
    block = 'water'
  []
  [speed]
    type = PINSFVSpeedFunctorMaterial
    porosity = 1
    superficial_vel_x = vel_x
    superficial_vel_y = vel_y
    superficial_vel_z = vel_z
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -sub_pc_factor_shift_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu  NONZERO superlu_dist'
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8
  line_search = none
[]

[Postprocessors]
  [p_in]
    type = SideAverageValue
    variable = pressure
    boundary = 'fluid-inlet'
  []
  [p_out]
    type = SideAverageValue
    variable = pressure
    boundary = 'fluid-outlet'
  []
  [delta_p]
    type = ParsedPostprocessor
    pp_names = 'p_in p_out'
    function = 'p_in - p_out'
  []
[]

[Outputs]
  exodus = true
[]

[Debug]
  show_var_residual_norms = true
[]

