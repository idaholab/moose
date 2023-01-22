# This test provides an example of combining two LPS viscoplasticity models with different stress
# exponents.

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
  xmax = 0.002
  ymax = 0.002
[]

[Variables]
  [./temp]
    initial_condition = 1000
  [../]
[]

[Kernels]
  [./dt]
    type = ADTimeDerivative
    variable = temp
  [../]
  [./diff]
    type = ADDiffusion
    variable = temp
  [../]
[]

[Modules/TensorMechanics/Master/All]
  strain = FINITE
  add_variables = true
  generate_output = 'strain_xx strain_yy strain_xy hydrostatic_stress vonmises_stress'
  use_automatic_differentiation = true
[]

[Functions]
  [./pull]
    type = PiecewiseLinear
    x = '0 0.1'
    y = '0 1e-5'
  [../]
  [./tot_effective_viscoplasticity]
    type = ParsedFunction
    symbol_values = 'lps_1_eff_creep_strain lps_3_eff_creep_strain'
    symbol_names = 'lps_1_eff_creep_strain lps_3_eff_creep_strain'
    expression = 'lps_1_eff_creep_strain+lps_3_eff_creep_strain'
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 1e10
    poissons_ratio = 0.3
  [../]
  [./stress]
    type = ADComputeMultipleInelasticStress
    inelastic_models = 'one two'
    outputs = all
  [../]
  [./porosity]
    type = ADPorosityFromStrain
    initial_porosity = 0.1
    inelastic_strain = 'combined_inelastic_strain'
    outputs = 'all'
  [../]

  [./one]
    type = ADViscoplasticityStressUpdate
    coefficient = 'coef_3'
    power = 3
    base_name = 'lps_1'
    outputs = all
    relative_tolerance = 1e-11
  [../]
  [./two]
    type = ADViscoplasticityStressUpdate
    coefficient = 1e-10
    power = 1
    base_name = 'lps_3'
    outputs = all
    relative_tolerance = 1e-11
  [../]
  [./coef]
    type = ADParsedMaterial
    property_name = coef_3
    # Example of creep power law
    coupled_variables = temp
    expression = '0.5e-18 * exp(-4e4 / 1.987 / temp)'
  [../]
[]

[BCs]
  [./no_disp_x]
    type = ADDirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
  [./no_disp_y]
    type = ADDirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  [../]
  [./pull_disp_y]
    type = ADFunctionDirichletBC
    variable = disp_y
    boundary = top
    function = pull
  [../]
  [./temp_ramp]
    type = ADFunctionDirichletBC
    boundary = right
    function = '1000 + 400 * t / 0.12'
    variable = temp
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  dt = 0.01
  end_time = 0.12
[]

[Postprocessors]
  [./disp_x]
    type = SideAverageValue
    variable = disp_x
    boundary = right
  [../]
  [./disp_y]
    type = SideAverageValue
    variable = disp_y
    boundary = top
  [../]
  [./avg_hydro]
    type = ElementAverageValue
    variable = hydrostatic_stress
  [../]
  [./avg_vonmises]
    type = ElementAverageValue
    variable = vonmises_stress
  [../]
  [./dt]
    type = TimestepSize
  [../]
  [./num_lin]
    type = NumLinearIterations
    outputs = console
  [../]
  [./num_nonlin]
    type = NumNonlinearIterations
    outputs = console
  [../]
  [./lps_1_eff_creep_strain]
    type = ElementAverageValue
    variable = lps_1_effective_viscoplasticity
  [../]
  [./lps_3_eff_creep_strain]
    type = ElementAverageValue
    variable = lps_3_effective_viscoplasticity
  [../]
  [./lps_1_gauge_stress]
    type = ElementAverageValue
    variable = lps_1_gauge_stress
  [../]
  [./lps_3_gauge_stress]
    type = ElementAverageValue
    variable = lps_3_gauge_stress
  [../]
  [./eff_creep_strain_tot]
    type = FunctionValuePostprocessor
    function = tot_effective_viscoplasticity
  [../]
  [./porosity]
    type = ElementAverageValue
    variable = porosity
  [../]
[]

[Outputs]
  csv = true
[]
