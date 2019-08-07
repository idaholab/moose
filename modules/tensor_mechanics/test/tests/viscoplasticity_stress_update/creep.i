[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  xmax = 0.002
  ymax = 0.002
[]

[Modules/TensorMechanics/Master/All]
  strain = FINITE
  add_variables = true
  base_name = 'asdf'
  generate_output = 'strain_xx strain_yy strain_xy hydrostatic_stress vonmises_stress'
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
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e10
    poissons_ratio = 0.3
    base_name = 'asdf'
  [../]
  [./porous_stress]
    type = ComputeMultiplePorousInelasticStress
    inelastic_models = creep
    initial_porosity = 0.1
    outputs = all
    base_name = 'asdf'
  [../]
  [./regular_stress]
    type = ComputeMultipleInelasticStress
    inelastic_models = creep
    outputs = all
    base_name = 'asdf'
  [../]
  [./porosity]
    type = GenericConstantMaterial
    prop_names = porosity
    prop_values = 0.1
    outputs = all
  [../]
  [./creep]
    type = PowerLawCreepStressUpdate
    activation_energy = 4e4
    temperature = 1200
    coefficient = 1e-18
    gas_constant = 1.987
    n_exponent = 3
    base_name = 'creep'
    outputs = all
  [../]
[]

[BCs]
  [./no_disp_x]
    type = PresetBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
  [./no_disp_y]
    type = PresetBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  [../]
  [./pull_disp_y]
    type = FunctionPresetBC
    variable = disp_y
    boundary = top
    function = pull
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
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
    variable = asdf_hydrostatic_stress
  [../]
  [./avg_vonmises]
    type = ElementAverageValue
    variable = asdf_vonmises_stress
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
    variable = creep_effective_creep_strain
  [../]
  [./porosity]
    type = ElementAverageValue
    variable = porosity
  [../]
[]

[Outputs]
  csv = true
[]
