[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
   file = heat_conduction_out.e
  [fmesh]
    type = FileMeshGenerator
    file = 'mesh_in.e'
  []
[]

[Variables]
  [temperature]
    initial_condition = 300
  []
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[Modules/TensorMechanics/Master]
  add_variables = true
  strain = FINITE
  temperature = temperature
  generate_output = 'stress_xx stress_yy stress_zz vonmises_stress
                     hydrostatic_stress elastic_strain_xx elastic_strain_yy
                     elastic_strain_zz strain_xx strain_yy strain_zz'
  [fuel_mechanics]
    block = 'fuel'
    eigenstrain_names = 'fuel_thermal_strain'
  []
  [clad_mechanics]
    block = 'clad'
    eigenstrain_names = 'clad_thermal_strain'
    generate_output = 'l2norm_plastic_strain'
  []
[]

[Kernels]
  [heat_conduction]
    type = HeatConduction
    variable = temperature
  []
  [heat_source]
    type = HeatSource
    block = fuel
    variable = temperature
    function = power_history
  []
[]

[Functions]
    [rho_UMo]
        type = ParsedFunction
        value = '-0.9215 * x + 17409'
    []
    [cp_UMo]
        type = ParsedFunction
        value = '0.0692 * x + 113.61'
    []
    [k_UMo]
        type = ParsedFunction
        value = '0.0413 * x + 0.1621'
    []

    [rho_Al]
        type = ParsedFunction
        value = '2702'
    []
    [cp_Al]
        type = ParsedFunction
        value = '3.97e-5 * x * x + 0.41 * x + 773'
    []
    [k_Al]
        type = ParsedFunction
        value = '-1.73e-7 * x * x * x + 2.66e-5 * x * x + 0.16 * x + 120.6'
    []

    [power_density]
      type = PiecewiseLinear
      data_file = 'POWER_DENSITY.csv' # W/cm3 to W/m3
      scale_factor = 1e5
      format = columns
    []
    [L2AR]
      type = ParsedFunction
      vars = 'x_offset y_offset'
      vals = '9.4615e-3 3.175e-3'
      value = 'x0 := x-x_offset;
              y0 := y-y_offset;
              (4.669e-10*x0^5 - 8.657e-8*x0^4 + 5.877e-6*x0^3 - 0.0001962*x0^2 + 0.004373*x0 + 0.9446) * (5.047e-7*y0^6 - 2.974e-05*y0^5 + 0.0006881*y0^4 - 0.007883*y0^3 + 0.04726*y0^2 - 0.1458*y0 + 1.152)'
    []
    [power_history]
      type = CompositeFunction
      functions = 'power_density L2AR'
    []
[]

[SolidProperties]
    [UMo_thermal]
        type = ThermalFunctionSolidProperties
        k = k_UMo
        cp = cp_UMo
        rho = rho_UMo
    []
    [Al_thermal]
        type = ThermalFunctionSolidProperties
        k = k_Al
        cp = cp_Al
        rho = rho_Al
    []
[]

[Materials]
    [UMo_thermal]
        type = ThermalSolidPropertiesMaterial
        sp = 'UMo_thermal'
        temperature = temperature
        block = 'fuel'
    []
    [Al_thermal]
        type = ThermalSolidPropertiesMaterial
        sp = 'Al_thermal'
        temperature = temperature
        block = 'clad'
    []
    [UMo_mechanical]
        type = ComputeIsotropicElasticityTensor
        poissons_ratio = 0.41
        youngs_modulus = 88.4e9
        block = 'fuel'
    []
    [Al_mechanical]
        type = ComputeIsotropicElasticityTensor
        poissons_ratio = 0.33
        youngs_modulus = 68.3e9
        block = 'clad'
    []
    [thermal_expansion_fuel]
        type = ComputeThermalExpansionEigenstrain
        stress_free_temperature = 300
        thermal_expansion_coeff = 13e-6
        temperature = temperature
        eigenstrain_name = fuel_thermal_strain
        block = 'fuel'
    []
    [thermal_expansion_clad]
        type = ComputeThermalExpansionEigenstrain
        stress_free_temperature = 300
        thermal_expansion_coeff = 23e-6
        temperature = temperature
        eigenstrain_name = clad_thermal_strain
        block = 'clad'
    [] 
    [stress1]
        type = ComputeFiniteStrainElasticStress
        block = 'fuel'
    []
    [stress]
        type = ComputeMultipleInelasticStress
        inelastic_models = 'isoplas'
        block = 'clad'
    []
    [isoplas]
        type = IsotropicPlasticityStressUpdate
        yield_stress = 276e6
        hardening_constant = 1.2e6
        block = 'clad'
    []
[]

[BCs]
  [walls]
    type = DirichletBC
    variable = temperature
    boundary = 'clad_wall'
    value = 300
  []
  [leftright_disp_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'bottom_to_clad top_to_clad'
    value = 0
  []
  [bottom_disp_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'bottom_to_clad top_to_clad'
    value = 0
  []
  [bottom_disp_z]
    type = DirichletBC
    variable = disp_z
    boundary = 'bottom_to_clad top_to_clad'
    value = 0
  []
  [top_presure_load]
    type = Pressure
    variable = disp_x
    boundary = 'top_plate_up bot_plate_up'
    factor = 1.2e6
  []
  [bot_presure_load]
    type = Pressure
    variable = disp_x
    boundary = 'top_plate_down bot_plate_down'
    factor = 1.1e6
  []
[]

[Executioner]
  type = Transient
  solve_type = 'Newton'

  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre    boomeramg      4'

  line_search = 'none'

  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-6
  nl_max_its = 50

  l_tol = 1e-4
  l_max_its = 50

  start_time = 0.0
  end_time = 4586400
  num_steps= 2500

  dtmax = 86400

  [Predictor]
    type = SimplePredictor
    scale = 1
  []

  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1800
    optimal_iterations = 10
    iteration_window = 2
    growth_factor = 2
    cutback_factor = .5
    timestep_limiting_function = power_density
    force_step_every_function_point = true
  []
[]

[Outputs]
  exodus = true
  csv = true
[]

[Postprocessors]

  [peak_temperature]
    type = NodalExtremeValue
    variable = temperature
  []

  [max_displacement_x]
    type = NodalExtremeValue
    variable = disp_x
    value_type = min
  []

  [max_displacement_y]
    type = NodalExtremeValue
    variable = disp_y
  []

  [max_displacement_z]
    type = NodalExtremeValue
    variable = disp_z
  []

  [plastic_strain]
    type = ElementAverageValue
    variable = 'l2norm_plastic_strain'
  []
[]