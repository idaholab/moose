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

[AuxVariables]
  [./hydrostatic_stress]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./hydrostatic_stress]
    type = ADRankTwoScalarAux
    variable = hydrostatic_stress
    rank_two_tensor = stress
    scalar_type = Hydrostatic
  [../]
[]

[Variables]
  [./disp_x]
    order = SECOND
    scaling = 1e-10
  [../]
  [./disp_y]
    order = SECOND
    scaling = 1e-10
  [../]
[]

[Functions]
  [./pull]
    type = PiecewiseLinear
    x = '0 10'
    y = '0 1e-3'
  [../]
[]

[Kernels]
  [./stress_x]
    type = ADStressDivergenceTensors
    component = 0
    variable = disp_x
  [../]
  [./stress_y]
    type = ADStressDivergenceTensors
    component = 1
    variable = disp_y
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 1e10
    poissons_ratio = 0.3
  [../]
  [./strain]
    type = ADComputeIncrementalStrain
  [../]
  [./elastic_strain]
    type = ADComputeMultipleInelasticStress
  [../]

  [./creep_ten]
    type = ADPowerLawCreepStressUpdate
    coefficient = 10e-24
    n_exponent = 4
    activation_energy = 0
    base_name = creep_ten
  [../]
  [./creep_ten2]
    type = ADPowerLawCreepStressUpdate
    coefficient = 10e-24
    n_exponent = 4
    activation_energy = 0
    base_name = creep_ten2
  [../]
  [./creep_one]
    type = ADPowerLawCreepStressUpdate
    coefficient = 1e-24
    n_exponent = 4
    activation_energy = 0
    base_name = creep_one
  [../]
  [./creep_nine]
    type = ADPowerLawCreepStressUpdate
    coefficient = 9e-24
    n_exponent = 4
    activation_energy = 0
    base_name = creep_nine
  [../]
  [./creep_zero]
    type = ADPowerLawCreepStressUpdate
    coefficient = 0e-24
    n_exponent = 4
    activation_energy = 0
    base_name = creep_zero
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

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient

  petsc_options_iname = -pc_hypre_type
  petsc_options_value = boomeramg

  line_search = 'none'
  nl_rel_tol = 1e-5

  num_steps = 5
  dt = 1e-1
[]

[Postprocessors]
  [./max_disp_x]
    type = ElementExtremeValue
    variable = disp_x
  [../]
  [./max_disp_y]
    type = ElementExtremeValue
    variable = disp_y
  [../]
  [./max_hydro]
    type = ElementAverageValue
    variable = hydrostatic_stress
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
[]

[Outputs]
  csv = true
[]
