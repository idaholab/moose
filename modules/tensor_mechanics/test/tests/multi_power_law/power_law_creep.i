[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  second_order = true
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = false
[]

[Functions]
  [pull]
    type = PiecewiseLinear
    x = '0 10'
    y = '0 1e-3'
  []
[]

[AuxVariables]
  [strain_energy_rate_density]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [strain_energy_rate_density]
    type = MaterialRealAux
    variable = strain_energy_rate_density
    property = strain_energy_rate_density
    execute_on = timestep_end
  []
[]

[Modules/TensorMechanics/Master]
  [MasterAction]
    strain = SMALL
    incremental = true
    add_variables = true
    use_automatic_differentiation = true
    generate_output = 'hydrostatic_stress vonmises_stress'
  []
[]

[Materials]
  [elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 1e10
    poissons_ratio = 0.3
  []
  [elastic_strain]
    type = ADComputeMultipleInelasticStress
    inelastic_models = "creep_nine creep_one"
  []
  [creep_one]
    type = ADPowerLawCreepStressUpdate
    coefficient = 1e-24
    n_exponent = 4
    m_exponent = 0
    activation_energy = 0
    base_name = creep_one
  []
  [creep_nine]
    type = ADPowerLawCreepStressUpdate
    coefficient = 9e-24
    n_exponent = 4
    m_exponent = 0
    activation_energy = 0
    base_name = creep_nine
  []
  [strain_energy_rate_density]
    type = ADStrainEnergyRateDensity
    inelastic_models = 'creep_nine'
  []
[]

[BCs]
  [no_disp_x]
    type = ADDirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  []

  [no_disp_y]
    type = ADDirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  []

  [pull_disp_y]
    type = ADFunctionDirichletBC
    variable = disp_y
    boundary = top
    function = pull
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient

  petsc_options_iname = -pc_hypre_type
  petsc_options_value = boomeramg

  line_search = 'none'
  nl_rel_tol = 1e-11
  nl_abs_tol = 1e-11
  num_steps = 5
  dt = 1e-1
[]

[Postprocessors]
  [max_disp_x]
    type = ElementExtremeValue
    variable = disp_x
  []
  [max_disp_y]
    type = ElementExtremeValue
    variable = disp_y
  []
  [max_hydro]
    type = ElementAverageValue
    variable = hydrostatic_stress
  []
  [dt]
    type = TimestepSize
  []
  [num_lin]
    type = NumLinearIterations
    outputs = console
  []
  [num_nonlin]
    type = NumNonlinearIterations
    outputs = console
  []
[]

[Outputs]
  exodus = true
[]
