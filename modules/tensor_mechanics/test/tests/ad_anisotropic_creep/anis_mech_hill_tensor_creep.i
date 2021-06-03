[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 128
  ny = 128
  second_order = true
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = false
[]

[AuxVariables]
  [hydrostatic_stress]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [hydrostatic_stress]
    type = ADRankTwoScalarAux
    variable = hydrostatic_stress
    rank_two_tensor = stress
    scalar_type = Hydrostatic
  []
[]

[Variables]
  [disp_x]
    order = SECOND
    scaling = 1e-10
  []
  [disp_y]
    order = SECOND
    scaling = 1e-10
  []
[]

[Functions]
  [pull]
    type = PiecewiseLinear
    x = '0 10e3'
    y = '0 1e-4'
  []
[]

[Kernels]
  [stress_x]
    type = ADStressDivergenceTensors
    component = 0
    variable = disp_x
  []
  [stress_y]
    type = ADStressDivergenceTensors
    component = 1
    variable = disp_y
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    strain = FINITE
    add_variables = true
    generate_output = 'elastic_strain_xx elastic_strain_yy elastic_strain_xy'
    use_automatic_differentiation = true
  []
[]

[Materials]
  [elasticity_tensor]
    type = ADComputeElasticityTensor
    fill_method = orthotropic
    C_ijkl = '2.0e3 2.0e5 2.0e3 0.71428571e3 0.71428571e3 0.71428571e3 0.4 0.2 0.004 0.004 0.2 0.4'
  []
  [elastic_strain]
    type = ADComputeMultipleInelasticStress
    inelastic_models = "trial_creep"
    max_iterations = 300
  []
  [hill_tensor]
    type = HillConstants
    # F G H L M N
    hill_constants = "0.5 0.5 0.3866 1.6413 1.6413 1.2731"
    base_name = trial_creep
  []
  [trial_creep]
    type = ADHillCreepStressUpdate
    coefficient = 1e-24
    n_exponent = 4
    m_exponent = 0
    activation_energy = 0
    # internal_solve_output_on = always
    base_name = trial_creep
  []

  [creep_one]
    type = ADPowerLawCreepStressUpdate
    coefficient = 1e-24
    n_exponent = 4
    activation_energy = 0
    base_name = creep_one
  []
  [creep_nine]
    type = ADPowerLawCreepStressUpdate
    coefficient = 9e-24
    n_exponent = 4
    activation_energy = 0
    base_name = creep_nine
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

  solve_type = Newton
  petsc_options_iname = '-ksp_gmres_restart -pc_type -sub_pc_type'
  petsc_options_value = '101                asm      lu'

  line_search = 'none'
  nl_rel_tol = 1e-5
  nl_abs_tol = 1.0e-13

  num_steps = 200
  dt = 1.0e2
  automatic_scaling = true
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
  csv = true
  exodus = true
[]
