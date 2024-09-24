# Units: specific_heat_capacity--cp--J/(kg.K); density--rho--kg/(cm^3);
# dynamic_viscosity--mu--kg/(cm.s); thermal_conductivity--k--W/(cm.K);
# pressure--kg/(cm.s^2); force--kg.cm/s^2

outlet_pressure = 0
inlet_velocity = 150 # cm/s
ini_temp = 593 # K
heat_transfer_coefficient = 9 # W/(cm2.K)
g = -981 # cm/s2
alpha_fluid = 2e-4 # thermal expansion coefficient of fluid used in INSADBoussinesqBodyForce

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  file = '2layers_2d_midline.msh'
[]

[Variables]
  [velocity]
    family = LAGRANGE_VEC
    order = FIRST
    block = 'fluid'
  []
  [p]
    family = LAGRANGE
    order = FIRST
    block = 'fluid'
  []
  [Tf]
    family = LAGRANGE
    order = FIRST
    block = 'fluid'
  []
  [Ts]
    family = LAGRANGE
    order = FIRST
    block = 'solid'
  []
  [disp_x]
    family = LAGRANGE
    order = FIRST
    block = 'solid fluid'
  []
  [disp_y]
    family = LAGRANGE
    order = FIRST
    block = 'solid fluid'
  []
[]

[AuxVariables]
  [heat_source]
    family = MONOMIAL
    order = FIRST
    block = 'solid'
  []
[]

[ICs]
  [initial_velocity]
    type = VectorConstantIC
    variable = velocity
    x_value = 0
    y_value = ${inlet_velocity}
    z_value = 0
  []
  [initial_p]
    type = FunctionIC
    variable = p
    function = ini_p
  []
  [initial_Tf]
    type = ConstantIC
    variable = Tf
    value = ${ini_temp}
  []
  [initial_Ts]
    type = ConstantIC
    variable = Ts
    value = ${ini_temp}
  []
[]

[Kernels]
  [fluid_mass]
    type = INSADMass
    variable = p
    use_displaced_mesh = true
  []
  [fluid_mass_pspg]
    type = INSADMassPSPG
    variable = p
    use_displaced_mesh = true
  []
  [fluid_momentum_time]
    type = INSADMomentumTimeDerivative
    variable = velocity
    use_displaced_mesh = true
  []
  [fluid_momentum_convection]
    type = INSADMomentumAdvection
    variable = velocity
    use_displaced_mesh = true
  []
  [fluid_momentum_viscous]
    type = INSADMomentumViscous
    variable = velocity
    use_displaced_mesh = true
  []
  [fluid_momentum_pressure]
    type = INSADMomentumPressure
    variable = velocity
    pressure = p
    integrate_p_by_parts = true
    use_displaced_mesh = true
  []
  [fluid_momentum_gravity]
    type = INSADGravityForce
    variable = velocity
    gravity = '0 ${g} 0'
    use_displaced_mesh = true
  []
  [fluid_momentum_buoyancy]
    type = INSADBoussinesqBodyForce
    variable = velocity
    gravity = '0 ${g} 0'
    alpha_name = 'alpha_fluid'
    ref_temp = 'T_ref'
    temperature = Tf
    use_displaced_mesh = true
  []
  [fluid_momentum_supg]
    type = INSADMomentumSUPG
    variable = velocity
    velocity = velocity
    use_displaced_mesh = true
  []
  [fluid_temperature_time]
    type = INSADHeatConductionTimeDerivative
    variable = Tf
    use_displaced_mesh = true
  []
  [fluid_temperature_conduction]
    type = ADHeatConduction
    variable = Tf
    thermal_conductivity = 'k'
    use_displaced_mesh = true
  []
  [fluid_temperature_advection]
    type = INSADEnergyAdvection
    variable = Tf
    use_displaced_mesh = true
  []
  [fluid_temperature_supg]
    type = INSADEnergySUPG
    variable = Tf
    velocity = velocity
    use_displaced_mesh = true
  []

  [solid_temperature_time]
    type = ADHeatConductionTimeDerivative
    variable = Ts
    density_name = 'rho'
    specific_heat = 'cp'
    block = 'solid'
    use_displaced_mesh = true
  []
  [solid_temperature_conduction]
    type = ADHeatConduction
    variable = Ts
    thermal_conductivity = 'k'
    block = 'solid'
    use_displaced_mesh = true
  []
  [heat_source]
    type = ADCoupledForce
    variable = Ts
    v = heat_source
    block = 'solid'
    use_displaced_mesh = true
  []

  [disp_x_smooth]
    type = Diffusion
    variable = disp_x
    block = fluid
  []
  [disp_y_smooth]
    type = Diffusion
    variable = disp_y
    block = fluid
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  strain = FINITE
  material_output_order = FIRST
  generate_output = 'vonmises_stress stress_xx stress_yy stress_zz strain_xx strain_yy strain_zz'
  [solid]
    block = 'solid'
    temperature = Ts
    automatic_eigenstrain_names = true
  []
[]

[InterfaceKernels]
  [convection_heat_transfer]
    type = ConjugateHeatTransfer
    variable = Tf
    T_fluid = Tf
    neighbor_var = 'Ts'
    boundary = 'solid_wall'
    htc = 'htc'
    use_displaced_mesh = true
  []
[]

[AuxKernels]
  [heat_source_distribution_auxk]
    type = FunctionAux
    variable = heat_source
    function = heat_source_distribution_function
    block = 'solid'
    use_displaced_mesh = true
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[BCs]
  [no_slip]
    type = VectorFunctionDirichletBC
    variable = velocity
    boundary = 'solid_wall'
    use_displaced_mesh = true
  []
  [inlet_velocity]
    type = VectorFunctionDirichletBC
    variable = velocity
    boundary = 'fluid_bottom'
    function_y = ${inlet_velocity}
    use_displaced_mesh = true
  []
  [symmetry]
    type = ADVectorFunctionDirichletBC
    variable = velocity
    boundary = 'fluid_wall'
    function_x = 0
    set_x_comp = true
    set_y_comp = false
    set_z_comp = false
    use_displaced_mesh = true
  []
  [outlet_p]
    type = DirichletBC
    variable = p
    boundary = 'fluid_top'
    value = ${outlet_pressure}
    use_displaced_mesh = true
  []
  [inlet_T]
    type = DirichletBC
    variable = Tf
    boundary = 'fluid_bottom'
    value = ${ini_temp}
    use_displaced_mesh = true
  []

  [pin1_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'pin1'
    value = 0
    use_displaced_mesh = true
  []
  [pin1_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'pin1'
    value = 0
    use_displaced_mesh = true
  []
  [top_and_bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'solid_bottom solid_top fluid_top fluid_bottom'
    value = 0
    use_displaced_mesh = true
  []
  [left_and_right_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'fluid_wall fluid_bottom'
    value = 0
    use_displaced_mesh = true
  []
[]

[Materials]
  [rho_solid]
    type = ADParsedMaterial
    property_name = rho
    expression = '0.0110876 * pow(9.9672e-1 + 1.179e-5 * Ts - 2.429e-9 * pow(Ts,2) + 1.219e-12 * pow(Ts,3),-3)'
    coupled_variables = 'Ts'
    block = 'solid'
    use_displaced_mesh = true
  []
  [cp_solid]
    type = ADParsedMaterial
    property_name = cp
    expression = '0.76 * ((302.27 * pow((548.68 / Ts),2) * exp(548.68 / Ts)) / pow((exp(548.68 / Ts) - 1),2) + 2 * 8.463e-3 * Ts + 8.741e7 * 18531.7 * exp(-18531.7 / Ts) / pow(Ts,2)) + 0.24 * ((322.49 * pow((587.41/Ts),2) * exp(587.41 / Ts)) / pow((exp(587.41 / Ts) - 1),2) + 2 * 1.4679e-2 * Ts)'
    coupled_variables = 'Ts'
    block = 'solid'
    use_displaced_mesh = true
  []
  [k_solid]
    type = ADParsedMaterial
    property_name = k
    expression = '1.158/(7.5408 + 17.692 * (Ts / 1000) + 3.6142 * pow((Ts/1000),2)) + 74.105 * pow((Ts / 1000),-2.5) * exp(-16.35 / (Ts / 1000))'
    coupled_variables = 'Ts'
    block = 'solid'
    use_displaced_mesh = true
  []

  [rho_fluid]
    type = ADParsedMaterial
    property_name = rho
    expression = '(11096 - 1.3236 * Tf) * 1e-6'
    coupled_variables = 'Tf'
    block = 'fluid'
    use_displaced_mesh = true
  []
  [cp_fluid]
    type = ADParsedMaterial
    property_name = cp
    expression = '159 - 2.72e-2 * Tf + 7.12e-6 * pow(Tf,2)'
    coupled_variables = 'Tf'
    block = 'fluid'
    use_displaced_mesh = true
  []
  [k_fluid]
    type = ADParsedMaterial
    property_name = k
    expression = '(3.61 + 1.517e-2 * Tf - 1.741e-6 * pow(Tf,2)) * 1e-2'
    coupled_variables = 'Tf'
    block = 'fluid'
    use_displaced_mesh = true
  []
  [mu_fluid]
    type = ADParsedMaterial
    property_name = mu
    expression = '4.94e-6 * exp(754.1/Tf)'
    coupled_variables = 'Tf'
    block = 'fluid'
    use_displaced_mesh = true
  []
  [buoyancy_thermal_expansion_coefficient_fluid]
    type = ADGenericConstantMaterial
    prop_names = 'alpha_fluid'
    prop_values = '${alpha_fluid}'
    block = 'fluid'
    use_displaced_mesh = true
  []
  [buoyancy_reference_temperature_fluid]
    type = GenericConstantMaterial
    prop_names = 'T_ref'
    prop_values = '${ini_temp}'
    block = 'fluid'
    use_displaced_mesh = true
  []

  [ins_mat_fluid]
    type = INSADStabilized3Eqn
    velocity = velocity
    pressure = p
    temperature = Tf
    block = 'fluid'
    use_displaced_mesh = true
  []

  [htc]
    type = ADGenericFunctionMaterial
    prop_names = htc
    prop_values = htc_function
    use_displaced_mesh = true
  []

  [elasticity_solid]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 2e7
    poissons_ratio = 0.32
    block = 'solid'
    use_displaced_mesh = true
  []
  [thermal_expansion_solid]
    type = ComputeThermalExpansionEigenstrain
    temperature = Ts
    thermal_expansion_coeff = 2e-4
    stress_free_temperature = 593
    eigenstrain_name = thermal_expansion
    block = 'solid'
    use_displaced_mesh = true
  []
  [stress_solid]
    type = ComputeFiniteStrainElasticStress
    block = 'solid'
  []
[]

[Functions]
  [htc_function]
    type = ParsedFunction
    expression = ${heat_transfer_coefficient}
  []
  [ini_p]
    type = ParsedFunction
    expression = '0.010302 * 981 * (10 - y)'
  []
  [heat_source_distribution_function]
    type = ParsedFunction
    expression = '300 * sin(pi * y / 10)'
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
    solve_type = 'PJFNK'
  []
[]

[Executioner]
  type = Transient
  end_time = 1e4

  solve_type = 'NEWTON'
  petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_linesearch_monitor'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
  line_search = 'none'

  nl_max_its = 30
  l_max_its = 100
  automatic_scaling = true
  compute_scaling_once = true
  off_diagonals_in_auto_scaling = true
  dtmin = 1
  nl_abs_tol = 1e-12

  [TimeStepper]
    type = IterationAdaptiveDT
    optimal_iterations = 6
    growth_factor = 1.5
    dt = 1
  []
[]

[Outputs]
  [csv]
    type = CSV
    file_base = 'thermal-me'
    execute_on = 'final'
  []
[]

[Postprocessors]
  [average_solid_Ts]
    type = ElementAverageValue
    variable = Ts
    block = 'solid'
    use_displaced_mesh = true
  []
  [average_fluid_Tf]
    type = ElementAverageValue
    variable = Tf
    block = 'fluid'
    use_displaced_mesh = true
  []
  [max_solid_Ts]
    type = ElementExtremeValue
    variable = Ts
    value_type = max
    block = 'solid'
    use_displaced_mesh = true
  []
  [max_fluid_Tf]
    type = ElementExtremeValue
    variable = Tf
    value_type = max
    block = 'fluid'
    use_displaced_mesh = true
  []
  [min_solid_Ts]
    type = ElementExtremeValue
    variable = Ts
    value_type = min
    block = 'solid'
    use_displaced_mesh = true
  []
  [min_fluid_Tf]
    type = ElementExtremeValue
    variable = Tf
    value_type = min
    block = 'fluid'
    use_displaced_mesh = true
  []
[]

[Debug]
  show_var_residual_norms = true
[]
