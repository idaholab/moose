# Process parameters
# scanning_speed=1.0 # m/s
power=25 # W (this is the effective power so multiplied by eta)
R=50e-6 # m (this is the effective radius)

# Geometric parameters
thickness=50e-6 # m
ymin=-180e-6
ymax=180e-6
xmin=-180e-6 # m
xmax=180e-6 # m
surfacetemp=300 # K (temperature at the other side of the plate)
backtemp=300

# Time stepping parameters
endtime=1.13e-3 # s
timestep=${fparse endtime/240} # s

[Mesh]
  [cmg]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = ${xmin}
    xmax = ${xmax}
    ymin = ${ymin}
    ymax = ${ymax}
    zmin = ${fparse -thickness}
    zmax = 0
    nx = 50
    ny = 50
    nz = 15
  []
[]

[Variables]
  [T]
  []
[]

[ICs]
  [T]
    type = FunctionIC
    variable = T
    function = '(${surfacetemp} - ${backtemp}) / ${thickness} * z + ${surfacetemp}'
  []
[]

[Kernels]
  [temperature_time]
    type = ADHeatConductionTimeDerivative
    variable = T
    use_displaced_mesh = true
    density_name = 'rho'
    specific_heat = 'cp'
  []
  [temperature_conduction]
    type = ADHeatConduction
    variable = T
    thermal_conductivity = 'k'
    use_displaced_mesh = true
  []
[]

[BCs]
  [T_cold]
    type = DirichletBC
    variable = T
    boundary = 'back'
    value = ${backtemp}
  []
  [radiation_flux]
    type = FunctionRadiativeBC
    variable = T
    boundary = 'front'
    emissivity_function = '1'
    Tinfinity = 300
    stefan_boltzmann_constant = 5.67e-8
  []
  [weld_flux]
    type = GaussianEnergyFluxBC
    variable = T
    boundary = 'front'
    P0 = ${power}
    R = ${R}
    x_beam_coord = xcoord
    y_beam_coord = ycoord
    z_beam_coord = '0'
  []
[]

[Functions]
  [xcoord]
    type = ParsedFunction
    expression = '60e-6*sin(pi/(60e-6*pi)*arclength)'
    symbol_names = 'arclength'
    symbol_values = 'laser_position'
  []
  [ycoord]
    type = ParsedFunction
    expression = '60e-6*cos(pi/(60e-6*pi)*arclength)'
    symbol_names = 'arclength'
    symbol_values = 'laser_position'
  []
  [reward_function]
    type = ParsedFunction
    expression = '1e-2*min(min(T1-1800, 0), 2800-T1)'
                 '+1e-2*min(min(T2-1800, 0), 2800-T2)'
                 '+1e-2*min(min(T3-1800, 0), 2800-T3)'
                 '+1e-2*min(min(T4-1800, 0), 2800-T4)'
                 '+1e-2*min(min(T5-1800, 0), 2800-T5)'
                 '+1e-2*min(min(T6-1800, 0), 2800-T6)'
                 '+1e-2*min(min(T7-1800, 0), 2800-T7)'
                 '+1e-2*min(min(T8-1800, 0), 2800-T8)'
    symbol_names = 'T1 T2 T3 T4 T5 T6 T7 T8'
    symbol_values = 'T1 T2 T3 T4 T5 T6 T7 T8'
  []
[]

[Postprocessors]
  [laser_position]
    type = LaserPositionPostprocessor
    execute_on = 'TIMESTEP_BEGIN'
    speed = speed_signal
  []
  [T1]
    type = PointValue
    variable = T
    point = '${fparse 60e-6*sin(2*pi*1/8)} ${fparse 60e-6*cos(2*pi*1/8)} 0.0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [T2]
    type = PointValue
    variable = T
    point = '${fparse 60e-6*sin(2*pi*2/8)} ${fparse 60e-6*cos(2*pi*2/8)} 0.0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [T3]
    type = PointValue
    variable = T
    point = '${fparse 60e-6*sin(2*pi*3/8)} ${fparse 60e-6*cos(2*pi*3/8)} 0.0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [T4]
    type = PointValue
    variable = T
    point = '${fparse 60e-6*sin(2*pi*4/8)} ${fparse 60e-6*cos(2*pi*4/8)} 0.0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [T5]
    type = PointValue
    variable = T
    point = '${fparse 60e-6*sin(2*pi*5/8)} ${fparse 60e-6*cos(2*pi*5/8)} 0.0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [T6]
    type = PointValue
    variable = T
    point = '${fparse 60e-6*sin(2*pi*6/8)} ${fparse 60e-6*cos(2*pi*6/8)} 0.0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [T7]
    type = PointValue
    variable = T
    point = '${fparse 60e-6*sin(2*pi*7/8)} ${fparse 60e-6*cos(2*pi*7/8)} 0.0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [T8]
    type = PointValue
    variable = T
    point = '${fparse 60e-6*sin(2*pi*8/8)} ${fparse 60e-6*cos(2*pi*8/8)} 0.0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [reward]
    type = FunctionValuePostprocessor
    function = reward_function
    execute_on = 'INITIAL TIMESTEP_END'
    indirect_dependencies = 'T1 T2 T3 T4 T5 T6 T7 T8'
  []
  [speed_signal]
    type = ConstantPostprocessor
    value = 1.0
    execute_on = TIMESTEP_BEGIN
  []
  [speed]
    type = LibtorchControlValuePostprocessor
    control_name = src_control
  []
  [log_prob_speed]
    type = LibtorchDRLLogProbabilityPostprocessor
    control_name = src_control
  []
[]

[Reporters]
  [results]
    type = AccumulateReporter
    reporters = 'T1/value T2/value T3/value T4/value T5/value T6/value T7/value T8/value reward/value speed/value log_prob_speed/value'
  []
[]

[Materials]
  [steel]
    type = LaserWeld316LStainlessSteel
    temperature = T
    use_constant_density = true
  []
[]

[Controls]
  [src_control]
    type = LibtorchDRLControl
    parameters = "Postprocessors/speed_signal/value"
    responses = 'T1 T2 T3 T4 T5 T6 T7 T8'

    # keep consistent with LibtorchDRLControlTrainer
    input_timesteps = 1
    response_shift_factors = '1500 1500 1500 1500 1500 1500 1500 1500'
    response_scaling_factors = '0.000666667 0.000666667 0.000666667 0.000666667 0.000666667 0.000666667 0.000666667 0.000666667'
    action_scaling_factors = 1.0

    # response_scaling_factors = '1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0'
    # response_shift_factors = '0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0'
    # action_scaling_factors = 1.0

    execute_on = 'TIMESTEP_BEGIN'
    smoother = 1.0
    num_stems_in_period = 10
    stochastic = true
  []
[]

[Executioner]
  type = Transient
  end_time = ${endtime}
  # dtmin = 1e-10
  # dtmax = 1e-5
  dt = ${timestep}
  # petsc_options_iname = '-pc_type -pc_factor_shift_type'
  # petsc_options_value = 'lu       NONZERO'
  petsc_options_iname = '-pc_type -pc_hypre_type -pc_factor_shift_type'
  petsc_options_value = 'hypre    boomeramg       NONZERO'
  petsc_options = '-snes_converged_reason -ksp_converged_reason -options_left'
  solve_type = 'NEWTON'
  line_search = 'none'
  nl_max_its = 5
  l_max_its = 100
  # [TimeStepper]
  #   type = IterationAdaptiveDT
  #   optimal_iterations = 5
  #   iteration_window = 1
  #   dt = ${timestep}
  #   linear_iteration_ratio = 1e6
  #   growth_factor = 1.25
  # []
[]

[Debug]
  show_var_residual_norms = true
[]

[Outputs]
  [exodus]
    type = Exodus
    # output_material_properties = true
  []
  console = false
[]
