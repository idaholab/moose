# Capped weak-plane plasticity, Kernel = PorousFlowPlasticHeatEnergy
[Mesh]
  type = GeneratedMesh
  dim = 3
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  PorousFlowDictator = dictator
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
  [temperature]
  []
[]

[ICs]
  [disp_x]
    type = RandomIC
    variable = disp_x
    min = -0.1
    max = 0.1
  []
  [disp_y]
    type = RandomIC
    variable = disp_y
    min = -0.1
    max = 0.1
  []
  [disp_z]
    type = RandomIC
    variable = disp_z
    min = -0.1
    max = 0.1
  []
  [temp]
    type = RandomIC
    variable = temperature
    min = 0.1
    max = 0.2
  []
[]

[Kernels]
  [phe]
    type = PorousFlowPlasticHeatEnergy
    variable = temperature
  []
  [dummy_disp_x]
    type = PorousFlowPlasticHeatEnergy
    coeff = -1.3
    variable = disp_x
  []
  [dummy_disp_y]
    type = PorousFlowPlasticHeatEnergy
    coeff = 1.1
    variable = disp_y
  []
  [dummy_disp_z]
    type = PorousFlowPlasticHeatEnergy
    coeff = 0.2
    variable = disp_z
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'temperature disp_x disp_y disp_z'
    number_fluid_phases = 0
    number_fluid_components = 0
  []
  [coh]
    type = TensorMechanicsHardeningExponential
    value_0 = 1
    value_residual = 2
    rate = 1
  []
  [tanphi]
    type = TensorMechanicsHardeningExponential
    value_0 = 1.0
    value_residual = 0.5
    rate = 2
  []
  [tanpsi]
    type = TensorMechanicsHardeningExponential
    value_0 = 0.1
    value_residual = 0.05
    rate = 3
  []
  [t_strength]
    type = TensorMechanicsHardeningExponential
    value_0 = 100
    value_residual = 100
    rate = 1
  []
  [c_strength]
    type = TensorMechanicsHardeningCubic
    value_0 = 1
    value_residual = 0
    internal_0 = -2
    internal_limit = 0
  []
[]

[Materials]
  [temp]
    type = PorousFlowTemperature
    temperature = temperature
  []
  [porosity]
    type = PorousFlowPorosity
    thermal = true
    mechanical = true
    porosity_zero = 0.3
    thermal_expansion_coeff = 1.3
  []
  [volstrain]
    type = PorousFlowVolumetricStrain
  []
  [phe]
    type = ComputePlasticHeatEnergy
  []
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    lambda = 1.0
    shear_modulus = 2.0
  []
  [strain]
    type = ComputeIncrementalSmallStrain
    displacements = 'disp_x disp_y disp_z'
    eigenstrain_names = ini_stress
  []
  [ini_stress]
    type = ComputeEigenstrainFromInitialStress
    initial_stress = '0 0 0  0 0 1  0 1 -1.5'
    eigenstrain_name = ini_stress
  []
  [admissible]
    type = ComputeMultipleInelasticStress
    inelastic_models = mc
    tangent_operator = nonlinear
  []
  [mc]
    type = CappedWeakPlaneStressUpdate
    cohesion = coh
    tan_friction_angle = tanphi
    tan_dilation_angle = tanpsi
    tensile_strength = t_strength
    compressive_strength = c_strength
    max_NR_iterations = 20
    tip_smoother = 0
    smoothing_tol = 1
    yield_function_tol = 1E-10
    perfect_guess = false
  []
[]

[Preconditioning]
  [andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -snes_type'
    petsc_options_value = 'bcgs bjacobi 1E-15 1E-10 10000 test'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
[]
