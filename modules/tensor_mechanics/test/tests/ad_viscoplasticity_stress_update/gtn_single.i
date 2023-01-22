# This test provides an example of an individual GTN viscoplasticity model

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

[Modules/TensorMechanics/Master/All]
  strain = FINITE
  add_variables = true
  base_name = 'total'
  generate_output = 'strain_xx strain_yy strain_xy hydrostatic_stress vonmises_stress'
  use_automatic_differentiation = true
[]

[Functions]
  [./pull]
    type = PiecewiseLinear
    x = '0 0.1'
    y = '0 1e-5'
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 1e10
    poissons_ratio = 0.3
    base_name = 'total'
  [../]
  [./stress]
    type = ADComputeMultipleInelasticStress
    inelastic_models = gtn
    outputs = all
    base_name = 'total'
  [../]
  [./porosity]
    type = ADPorosityFromStrain
    initial_porosity = 0.1
    inelastic_strain = 'total_combined_inelastic_strain'
    outputs = 'all'
  [../]

  [./gtn]
    type = ADViscoplasticityStressUpdate
    total_strain_base_name = 'total'
    coefficient = 'coef'
    power = 3
    viscoplasticity_model = GTN
    outputs = all
    relative_tolerance = 1e-11
  [../]
  [./coef]
    type = ADParsedMaterial
    property_name = coef
    # Example of creep power law
    expression = '1e-18 * exp(-4e4 / 1.987 / 1200)'
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
    variable = total_hydrostatic_stress
  [../]
  [./avg_vonmises]
    type = ElementAverageValue
    variable = total_vonmises_stress
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
  [./eff_creep_strain]
    type = ElementAverageValue
    variable = effective_viscoplasticity
  [../]
  [./porosity]
    type = ElementAverageValue
    variable = porosity
  [../]
[]

[Outputs]
  csv = true
[]
