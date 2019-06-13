[GlobalParams]
  displacements = 'disp_r disp_z'
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 100
  ymax = 0.304 # Length of test chamber
  xmax = 0.0257 # Test chamber radius
[]

[Variables]
  [pressure]
    scaling = 1e11
  []
  [temperature]
    initial_condition = 300 # Start at room temperature
  []
[]

[AuxVariables]
  [k_eff]
    initial_condition = 15.0 # water at 20C
  []
  [velocity_x]
    order = CONSTANT
    family = MONOMIAL
  []
  [velocity_y]
    order = CONSTANT
    family = MONOMIAL
  []
  [velocity_z]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    # This block adds all of the proper Kernels, strain calculators, and Variables
    # for TensorMechanics in the correct coordinate system (autodetected)
    add_variables = true
    strain = FINITE
    eigenstrain_names = eigenstrain
    generate_output = 'vonmises_stress elastic_strain_xx elastic_strain_yy strain_xx strain_yy'
  []
[]

[Kernels]
  [darcy_pressure]
    type = DarcyPressure
    variable = pressure
  []
  [heat_conduction]
    type = ADHeatConduction
    variable = temperature
  []
  [heat_conduction_time_derivative]
    type = ADHeatConductionTimeDerivative
    variable = temperature
  []
  [heat_convection]
    type = DarcyAdvection
    variable = temperature
    pressure = pressure
  []
[]

[AuxKernels]
  [velocity_x]
    type = DarcyVelocity
    variable = velocity_x
    component = x
    execute_on = timestep_end
    pressure = pressure
  []
  [velocity_y]
    type = DarcyVelocity
    variable = velocity_y
    component = y
    execute_on = timestep_end
    pressure = pressure
  []
  [velocity_z]
    type = DarcyVelocity
    variable = velocity_z
    component = z
    execute_on = timestep_end
    pressure = pressure
  []
[]

[BCs]
  [inlet]
    type = DirichletBC
    variable = pressure
    boundary = bottom
    value = 4000 # (Pa) From Figure 2 from paper.  First data point for 1mm spheres.
  []
  [outlet]
    type = DirichletBC
    variable = pressure
    boundary = top
    value = 0 # (Pa) Gives the correct pressure drop from Figure 2 for 1mm spheres
  []
  [inlet_temperature]
    type = FunctionDirichletBC
    variable = temperature
    boundary = bottom
    function = 'if(t<0,350+50*t,350)'
  []
  [outlet_temperature]
    type = HeatConductionOutflow
    variable = temperature
    boundary = top
  []
  [hold_inlet]
    type = DirichletBC
    variable = disp_z
    boundary = bottom
    value = 0
  []
  [hold_center]
    type = DirichletBC
    variable = disp_r
    boundary = left
    value = 0
  []
  [hold_outside]
    type = DirichletBC
    variable = disp_r
    boundary = right
    value = 0
  []
[]

[Materials]
  viscosity_file = data/water_viscosity.csv
  density_file = data/water_density.csv
  specific_heat_file = data/water_specific_heat.csv
  [column]
    type = PackedColumn
    temperature = temperature
    radius = 1
    thermal_conductivity = k_eff # Use the AuxVariable instead of calculating
    fluid_viscosity_file = ${viscosity_file}
    fluid_density_file = ${density_file}
    fluid_specific_heat_file = ${specific_heat_file}
  []

  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 200e9 # (Pa) from wikipedia
    poissons_ratio = .3 # from wikipedia
  []
  [elastic_stress]
    type = ComputeFiniteStrainElasticStress
  []
  [thermal_strain]
    type = ComputeThermalExpansionEigenstrain
    stress_free_temperature = 300
    thermal_expansion_coeff = 1e-6
    eigenstrain_name = eigenstrain
    temperature = temperature
  []
[]

[Postprocessors]
  [average_temperature]
    type = ElementAverageValue
    variable = temperature
  []
[]

[Problem]
  type = FEProblem
  coord_type = RZ
[]

[Executioner]
  type = Transient
  start_time = -1
  end_time = 200
  steady_state_tolerance = 8e-6
  steady_state_detection = true
  dt = 0.25
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 100'
  line_search = none
  nl_rel_tol = 1e-7
  [TimeStepper]
    type = FunctionDT
    function = 'if(t<0,0.1,0.25)'
  []
[]

[MultiApps]
  [micro]
    type = TransientMultiApp
    app_type = DarcyThermoMechApp
    positions = '0.01285 0.0    0
                 0.01285 0.0608 0
                 0.01285 0.1216 0
                 0.01285 0.1824 0
                 0.01285 0.2432 0
                 0.01285 0.304  0'
    input_files = step10_micro.i
    execute_on = 'timestep_end'
  []
[]

[Transfers]
  [keff_from_sub]
    type = MultiAppPostprocessorInterpolationTransfer
    direction = from_multiapp
    multi_app = micro
    variable = k_eff
    power = 1
    postprocessor = k_eff
    execute_on = 'timestep_end'
  []
  [temperature_to_sub]
    type = MultiAppVariableValueSamplePostprocessorTransfer
    direction = to_multiapp
    multi_app = micro
    source_variable = temperature
    postprocessor = temperature_in
    execute_on = 'timestep_end'
  []
[]

[Controls]
  [multiapp]
    type = TimePeriod
    disable_objects = 'MultiApps::micro Transfers::keff_from_sub Transfers::temperature_to_sub'
    start_time = '0'
    execute_on = 'initial'
  []
[]

[Outputs]
  [out]
    type = Exodus
    elemental_as_nodal = true
  []
[]
