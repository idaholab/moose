[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[AuxVariables]
  [temperature]
    initial_condition = 889
  []
  [effective_inelastic_strain]
    order = FIRST
    family = MONOMIAL
  []
  [cell_dislocations]
    order = FIRST
    family = MONOMIAL
  []
  [wall_dislocations]
    order = FIRST
    family = MONOMIAL
  []
  [number_of_substeps]
    order = FIRST
    family = MONOMIAL
  []
[]

[AuxKernels]
  [effective_inelastic_strain]
    type = MaterialRealAux
    variable = effective_inelastic_strain
    property = effective_creep_strain
  []
  [cell_dislocations]
    type = MaterialRealAux
    variable = cell_dislocations
    property = cell_dislocations
  []
  [wall_dislocations]
    type = MaterialRealAux
    variable = wall_dislocations
    property = wall_dislocations
  []
  [number_of_substeps]
    type = MaterialRealAux
    variable = number_of_substeps
    property = number_of_substeps
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    strain = FINITE
    add_variables = true
    generate_output = 'vonmises_stress'
  []
[]

[BCs]
  [symmy]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
  [symmx]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  []
  [symmz]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0
  []
  [pressure_x]
    type = Pressure
    variable = disp_x
    boundary = right
    factor = -0.5
    function = shear_function
  []
  [pressure_y]
    type = Pressure
    variable = disp_y
    boundary = top
    factor = -0.5
    function = shear_function
  []
  [pressure_z]
    type = Pressure
    variable = disp_z
    boundary = front
    factor = 0.5
    function = shear_function
  []
[]

[Functions]
  [shear_function]
    type = ParsedFunction
    expression = 'timeToDoubleInHours := 10;
            if(t<=28*60*60, 15.0e6, 15.0e6*(t-28*3600)/3600/timeToDoubleInHours+15.0e6)'
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1.68e11
    poissons_ratio = 0.31
  []
  [stress]
    type = ComputeMultipleInelasticStress
    inelastic_models = rom_stress_prediction

  []
  [mx_phase_fraction]
    type = GenericConstantMaterial
    prop_names = mx_phase_fraction
    prop_values = 5.13e-2 #precipitation bounds: 6e-3, 1e-1
    outputs = all
  []

  [rom_stress_prediction]
    type = SS316HLAROMANCEStressUpdateTest
    temperature = temperature
    initial_cell_dislocation_density = 6.0e12
    initial_wall_dislocation_density = 4.4e11

    use_substepping = NONE
    max_inelastic_increment = 0.0001

    stress_input_window_low_failure = WARN
    stress_input_window_high_failure = ERROR
    cell_input_window_high_failure = ERROR
    cell_input_window_low_failure = ERROR
    wall_input_window_low_failure = ERROR
    wall_input_window_high_failure = ERROR
    temperature_input_window_high_failure = ERROR
    temperature_input_window_low_failure = ERROR
    environment_input_window_high_failure = ERROR
    environment_input_window_low_failure = ERROR
  []
[]

[Executioner]
  type = Transient

  solve_type = 'NEWTON'

  nl_abs_tol = 1e-12
  nl_rel_tol = 1e-4
  automatic_scaling = true
  compute_scaling_once = false

  dtmin = 0.1
  dtmax = 1e5
  end_time = 136800
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 0.1 ## This model requires a tiny timestep at the onset for the first 10s
    iteration_window = 4
    optimal_iterations = 12
    time_t = '100800'
    time_dt = '1e5'
  []

[]

[Postprocessors]
  [effective_strain_avg]
    type = ElementAverageValue
    variable = effective_inelastic_strain
  []
  [temperature]
    type = ElementAverageValue
    variable = temperature
  []
  [cell_dislocations]
    type = ElementAverageValue
    variable = cell_dislocations
  []
  [wall_disloactions]
    type = ElementAverageValue
    variable = wall_dislocations
  []
  [max_vonmises_stress]
    type = ElementExtremeValue
    variable = vonmises_stress
    value_type = max
  []
  [number_of_substeps]
    type = ElementAverageValue
    variable = number_of_substeps
  []
[]

[Outputs]
  csv = true
[]
