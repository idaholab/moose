[GlobalParams]
  displacements = 'disp_r disp_z'
[]

[Problem]
  coord_type = RZ
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 1
  xmax = 2
  nx = 50
  ny = 50
[]

[Modules/TensorMechanics/Master]
  [all]
    strain = FINITE
    incremental = true
    add_variables = true
    eigenstrain_names = 'thermal'
    use_automatic_differentiation = true
  []
[]

[AuxVariables]
  [temp]
    initial_condition = 1000.0
  []
[]

[AuxKernels]
  [cooling]
    type = FunctionAux
    variable = temp
    function = '1000-10*t*x'
  []
[]

[BCs]
  [top_pull]
    type = ADFunctionNeumannBC
    variable = disp_z
    boundary = top
    function = '1e7*t'
    use_displaced_mesh = true
  []
  [bottom_fix]
    type = ADDirichletBC
    variable = disp_z
    boundary = bottom
    value = 0.0
  []
  [left_fix]
    type = ADDirichletBC
    variable = disp_r
    boundary = left
    value = 0.0
  []
[]

[Materials]
  [eigenstrain]
    type = ADComputeThermalExpansionEigenstrain
    eigenstrain_name = 'thermal'
    stress_free_temperature = 1000
    thermal_expansion_coeff = 1e-4
    temperature = temp
  []
  [elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 2e11
    poissons_ratio = 0.3
  []
  [stress]
    type = ADComputeMultipleInelasticStress
    inelastic_models = 'creep'
  []
  [creep]
    type = ADPowerLawCreepStressUpdate
    coefficient = 1.0e-15
    n_exponent = 4
    activation_energy = 3.0e5
    temperature = temp
  []
[]

[Postprocessors]
  [nl_its]
    type = NumNonlinearIterations
  []
  [total_nl_its]
    type = CumulativeValuePostprocessor
    postprocessor = nl_its
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  line_search = 'none'
  end_time = 10
  dt = 1

  automatic_scaling = true
[]

[Outputs]
  print_linear_converged_reason = false
  print_nonlinear_converged_reason = false
  print_linear_residuals = false
  perf_graph = true
[]
